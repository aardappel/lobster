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

#include "lobster/stdafx.h"

#include "lobster/vmdata.h"
#include "lobster/glinterface.h"
#include "lobster/glincludes.h"

// NOTE: Time queries require OpenGL 3.3 or otherwise GL_ARB_timer_query
// In case this causes problems, make sure to check platform availability

TimeQuery::~TimeQuery() {
    if (active) {
        GL_CALL(glDeleteQueries(2, query_buffer_ids[0]));
        GL_CALL(glDeleteQueries(2, query_buffer_ids[1]));
        active = !active;
    }
}

TimeQuery CreateTimeQuery() {
    return TimeQuery();
}

void TimeQuery::Start() {
    if (!active) {
        GL_CALL(glGenQueries(2, query_buffer_ids[0]));
        GL_CALL(glGenQueries(2, query_buffer_ids[1]));
        // Otherwise opengl reports errors?
        GL_CALL(glQueryCounter(query_buffer_ids[front_buffer_index][0], GL_TIMESTAMP));
        GL_CALL(glQueryCounter(query_buffer_ids[front_buffer_index][1], GL_TIMESTAMP));
        active = !active;
    }
    glQueryCounter(query_buffer_ids[back_buffer_index][0], GL_TIMESTAMP);
}

void TimeQuery::Stop() {
    if (!active) return;
    glQueryCounter(query_buffer_ids[back_buffer_index][1], GL_TIMESTAMP);
}

double TimeQuery::Evaluate() {
    if (!active) return 0.0;

    // TODO: average over 16-32 frames

    // Retrieve timings
    GLuint64 start = 0;
    GLuint64 end = 0;
    glGetQueryObjectui64v(query_buffer_ids[front_buffer_index][0], GL_QUERY_RESULT, &start);
    glGetQueryObjectui64v(query_buffer_ids[front_buffer_index][1], GL_QUERY_RESULT, &end);

    // Flip buffer indices
    front_buffer_index = 1 - front_buffer_index;
    back_buffer_index = 1 - back_buffer_index;

    // Convert into ms
    return (end - start) / 1000000.0;
}
