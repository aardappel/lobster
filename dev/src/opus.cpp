// Copyright 2026 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "opus.h"

using namespace lobster;

ResourceType opus_encoder_type = { "opusencoder" };
ResourceType opus_decoder_type = { "opusdecoder" };

struct Encoder : lobster::Resource {
    unique_ptr<OpusEncoder, decltype(&opus_encoder_destroy)> encoder;
    int channels;
    vector<float> samples;

    Encoder(OpusEncoder *encoder, int channels) : encoder(encoder, opus_encoder_destroy), channels(channels) {}
};

Encoder &GetEncoder(Value res) {
    return GetResourceDec<Encoder>(res, &opus_encoder_type);
}

struct Decoder : lobster::Resource {
    unique_ptr<OpusDecoder, decltype(&opus_decoder_destroy)> decoder;
    int channels;
    vector<string> packets;

    Decoder(OpusDecoder *decoder, int channels)
        : decoder(decoder, opus_decoder_destroy), channels(channels) {}
};

Decoder &GetDecoder(Value res) {
    return GetResourceDec<Decoder>(res, &opus_decoder_type);
}

void AddOpus(NativeRegistry &nfr) {

nfr("encoder_create", "sample_rate,channels", "II", "R:opusencoder",
    "create an opus encoder object with the given sample rate and channel count."
    " opus requires that the sample rate be: 8000, 12000, 16000, 24000, or 48000."
    " the channel count must be either 1 or 2",
    [](StackPtr &, VM &vm, Value sample_rate, Value channels) {
        int error;
        auto *opus_encoder = opus_encoder_create(sample_rate.intval(), channels.intval(),
                                                 OPUS_APPLICATION_VOIP, &error);
        if (error != OPUS_OK) {
            LOG_ERROR("opus_encoder_create: ", opus_strerror(error));
            return NilVal();
        }
        auto *encoder = new Encoder(opus_encoder, channels.intval());
        return Value(vm.NewResource(&opus_encoder_type, encoder));
    });

nfr("encode_float", "encoder,data,frame_size", "R:opusencoderF]I", "S]",
    "encode float data in the range [-1.0, 1.0] into packets."
    " opus requires that the frame_size be: 2.5ms, 5ms, 10ms, 20ms, 40ms, 60ms"
    " (e.g. at 48kHz the permitted values are 120, 240, 480, 960, 1920, 2880)",
    [](StackPtr &, VM &vm, Value vencoder, Value vdata, Value vframe_size) {
        auto &e = GetEncoder(vencoder);
        e.samples.reserve(e.samples.size() + vdata.vval()->len);
        for (int i = 0; i < vdata.vval()->len; i++)
            e.samples.push_back(vdata.vval()->AtS(i).fltval());
        auto frame_size = vframe_size.intval();
        auto frames = e.samples.size() / (frame_size * e.channels);
        auto *out = (LVector *)vm.NewVec(0, frames, TYPE_ELEM_VECTOR_OF_STRING);
        unsigned char packet_buffer[4000];  // TODO: big enough?
        for (int i = 0; i < (int)frames; ++i) {
            int bytes =
                opus_encode_float(e.encoder.get(), e.samples.data() + i * frame_size * e.channels,
                                  frame_size, packet_buffer, sizeof(packet_buffer));
            if (bytes < 0) {
                LOG_ERROR("opus_encode_float: ", opus_strerror(bytes));
                return Value(out);
            }
            out->Push(vm, vm.NewString(string_view((const char *)packet_buffer, bytes)));
        }
        e.samples.erase(e.samples.begin(), e.samples.begin() + frames * frame_size * e.channels);
        return Value(out);
    });

nfr("decoder_create", "sample_rate,channels", "II", "R:opusdecoder",
    "create an opus encoder object with the given sample rate and channel count."
    " opus requires that the sample rate be: 8000, 12000, 16000, 24000, or 48000."
    " the channel count must be either 1 or 2. "
    " this does not have to match the encoded sample rate and channel count.",
    [](StackPtr &, VM &vm, Value sample_rate, Value channels) {
        int error;
        auto *opus_decoder = opus_decoder_create(sample_rate.intval(), channels.intval(), &error);
        if (error != OPUS_OK) {
            LOG_ERROR("opus_decoder_create: ", opus_strerror(error));
            return NilVal();
        }
        auto *decoder = new Decoder(opus_decoder, channels.intval());
        return Value(vm.NewResource(&opus_decoder_type, decoder));
    });

nfr("decode_float", "decoder,packets,frame_size", "R:opusdecoderS]I", "F]",
    "decode opus-encoded packets."
    " opus requires that the frame_size be: 2.5ms, 5ms, 10ms, 20ms, 40ms, 60ms"
    " (e.g. at 48kHz the permitted values are 120, 240, 480, 960, 1920, 2880)"
    " the frame size does not have to match the encoded frame size.",
    [](StackPtr &, VM &vm, Value vdecoder, Value vpackets, Value vframe_size) {
        auto &d = GetDecoder(vdecoder);
        d.packets.reserve(d.packets.size() + vpackets.vval()->len);
        for (int i = 0; i < vpackets.vval()->len; i++)
            d.packets.push_back(string(vpackets.vval()->AtS(i).sval()->strv()));
        auto frame_size = vframe_size.intval();
        auto frames = d.packets.size();
        auto out = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_FLOAT);
        vector<float> pcm(frame_size * d.channels);
        for (int i = 0; i < (int)frames; ++i) {
            auto &packet = d.packets[i];
            int samples_per_channel =
                opus_decode_float(d.decoder.get(), (const unsigned char *)packet.data(),
                                  (int)packet.size(), pcm.data(), frame_size, 0);
            if (samples_per_channel < 0) {
                LOG_ERROR("opus_decode_float: ", opus_strerror(samples_per_channel));
                return NilVal();
            }
            for (int j = 0; j < samples_per_channel * d.channels; ++j) {
                out->Push(vm, pcm[j]);
            }
        }
        d.packets.erase(d.packets.begin(), d.packets.begin() + frames);
        return Value(out);
    });

}
