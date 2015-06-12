// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
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

struct VMLog
{
    bool uses_frame_state;
    vector<Value> logread, logwrite;
    size_t logi;
    const int *lognew;

    VM &vm;

    VMLog(VM &_vm, bool _ufs) : uses_frame_state(_ufs), logi(0), lognew(nullptr), vm(_vm) {}

    void LogInit()
    {
        if (uses_frame_state)
        {
            // get the logs in a good state before the first frame
            logread.push_back(Value(0, V_LOGMARKER));
            logread.push_back(Value(0, V_LOGEND));
            logread.push_back(Value(0, V_LOGMARKER));
            logi = 1;

            logwrite.push_back(Value(0, V_LOGMARKER));
            logwrite.push_back(Value(0, V_LOGEND));
        }
    }

    void LogFrame()
    {
        if (uses_frame_state)
        {
            while (logread[logi].type == V_LOGSTART) LogSkipNestedFuns();

            // the start of whatever called this frame
            assert(logwrite.back().type == V_LOGSTART);
            logwrite.pop_back();

            // always bookend the log with markers, so we can blindly look ahead/behind
            logwrite.push_back(Value(0, V_LOGMARKER));
            logwrite.swap(logread);
            logwrite.clear();
            logwrite.push_back(Value(0, V_LOGMARKER));
            logi = 1;
            lognew = nullptr;

            #ifdef _DEBUG
                #if 0
                    PrintPrefs pp(1, 20, true, -1);
                    printf("frame log:");
                    for (size_t i = logi + 1; i < logread.size() - 1; i++)
                    {
                        switch (logread[i].type)
                        {
                        case V_LOGSTART: printf(" ("); break;
                        case V_LOGEND: printf(")"); break;
                        case V_LOGMARKER: assert(0); break;
                        default: printf(" %s", logread[i].ToString(pp).c_str()); break;
                        }
                    }
                    printf("\n");
                #endif
            #endif
        }
    };

    void LogSkipNestedFuns()
    {
        logi++;
        int nest = 1;
        while (nest)
        {
            switch (logread[logi].type)
            {
                case V_LOGSTART: nest++; break;
                case V_LOGEND: nest--; break;
                case V_LOGMARKER: assert(0); break;
                default: logread[logi].DEC(); break;
            }
            logi++;
        }
    }

    size_t LogFunctionEntry(const int *funstart, int nlogvars)
    {
        assert(uses_frame_state);

        size_t lws = logwrite.size();

        logwrite.push_back(Value(funstart, V_LOGSTART));

        for (int i = 0; i < nlogvars; i++) logwrite.push_back(Value());

        if (!lognew)
        {
            if (logread[logi].type == V_LOGSTART && logread[logi].ip() == funstart)
            {
                logi++; // expected path: function present
                logi += nlogvars; // skip past them, read by index
            }
            else
            {
                lognew = funstart; // stop tryint to read from the log until we exit this function
            }
        }

        return lws;
    }

    void LogFunctionExit(const int *funstart, const int *logvars, int logfunwritestart)
    {
        if (logwrite.back().type == V_LOGSTART)
        {
            // common case: function didn't write anything, we cull it
            assert(logwrite.back().ip() == funstart);
            logwrite.pop_back();
        }
        else
        {
            int nlogvars = *logvars;
            logvars -= nlogvars;
            for (int i = 0; i < nlogvars; i++)
            {
                logwrite[i + logfunwritestart + 1] = vm.vars[*logvars++].INC();
            }
            logwrite.push_back(Value(funstart, V_LOGEND));
        }

        if (lognew)
        {
            if (lognew == funstart) lognew = nullptr;
        }
        else for (;;) switch (logread[logi].type)
        {
            case V_LOGEND:      // expected
                assert(logread[logi].ip() == funstart || !logread[logi].ip());
                logi++;
            case V_LOGMARKER:   // can happen with empty log
                return;

            case V_LOGSTART:    // clean up any
                LogSkipNestedFuns();
                break;

            default:            // should not happen
                assert(0);
                logread[logi].DEC();
                logi++;
                break;
        }
    }

    Value LogGet(Value def, int idx)
    {  
        assert(uses_frame_state);
        if (lognew)
        {
            return def;
        }
        else
        {
            Value &lfr = vm.stack[vm.sp - 5];   // depends upon what's written in FunIntro
            assert(lfr.type == V_LOGFUNREADSTART);

            def.DEC();
            return logread[lfr.ival() + idx];
        }
    }

    void LogCleanup()
    {
        for (auto &v : logread) v.DEC();
        for (auto &v : logwrite) v.DEC();
    }

    void LogMark()
    {
        for (auto &v : logread) v.Mark();
        for (auto &v : logwrite) v.Mark();
    }
};
