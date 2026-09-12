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

#include "lobster/compiler.h"

namespace lobster {

void DumpBuiltinNames(NativeRegistry &nfr) {
    string s;
    for (auto nf : nfr.nfuns) {
        if (nfr.subsystems[nf->subsystemid] == "plugin") continue;
        s += nf->name;
        s += "|";
    }
    WriteFile("builtin_functions_names.txt", false, s, false);
}

string JSONEscape(string_view in) {
    string s;
    flatbuffers::EscapeString(in.data(), in.size(), &s, false, false);
    return s.substr(1, s.size()-2);
}
string HTMLEscape(string_view in) {
    string s;
    for (auto c : in) {
        switch (c) {
            case '&': s += "&amp;"; break;
            case '<': s += "&lt;"; break;
            case '>': s += "&gt;"; break;
            default: s += c; break;
        }
    }
    return s;
}

enum Tags {
    Doc = 0,
    Table = 1,
    Row = 2,
    FirstRow = 3,
    Subsystem = 4,
    Name = 5,
    Params = 6,
    Help = 7,
    Font = 8,
    Returns = 9,
    ParamType = 10,
    ParamName = 11,
    ParamDefault = 12,
    Param = 13,
    RetTypeWrap = 14,
    NumTags
};

string GetBuiltinDoc(NativeRegistry &nfr, bool group_subsystem, string (&doc_tags)[NumTags][2], string (*escape)(string_view)) {
    string s = doc_tags[Tags::Doc][0];
    int cursubsystem = -1;
    bool is_first_row = true;
    bool tablestarted = !group_subsystem;
    if(tablestarted) s += doc_tags[Tags::Table][0];
    for (auto nf : nfr.nfuns) {
        if (nfr.subsystems[nf->subsystemid] == "plugin") continue;
        if (group_subsystem) {
            if (nf->subsystemid != cursubsystem) {
                if (tablestarted) s += doc_tags[Tags::Table][1];
                tablestarted = false;
                s += is_first_row ? doc_tags[Tags::FirstRow][0] : doc_tags[Tags::Row][0];
                s += cat(doc_tags[Tags::Subsystem][0], nfr.subsystems[nf->subsystemid], doc_tags[Tags::Subsystem][1]);
                s += doc_tags[Tags::Row][1];
                cursubsystem = nf->subsystemid;
            }
            if (!tablestarted) {
                s += doc_tags[Tags::Table][0];
                tablestarted = true;
            }
        }
        s += is_first_row ? doc_tags[Tags::FirstRow][0] : doc_tags[Tags::Row][0];
        if (!group_subsystem) {
            s += cat(doc_tags[Tags::Subsystem][0],
                    nfr.subsystems[nf->subsystemid],
                    doc_tags[Tags::Subsystem][1]);
        }
        s += cat(doc_tags[Tags::Name][0], nf->name, doc_tags[Tags::Name][1]);
        s += doc_tags[Tags::Params][0];
        int last_not_optional = -1;
        for (auto [i, a] : enumerate(nf->args)) {
            if (!a.optional) last_not_optional = (int)i;
        }
        for (auto [i, a] : enumerate(nf->args)) {
            auto argname = nf->args[i].name;
            if (i) s +=  ", ";
            s += doc_tags[Tags::Param][0];
            s += cat(doc_tags[Tags::ParamName][0], argname, doc_tags[Tags::ParamName][1]);
            s += doc_tags[Tags::Font][0];
            s += doc_tags[Tags::ParamType][0];
            if (a.vttype->t != V_ANY) {
                s += a.flags & NF_BOOL
                    ? "bool"
                    : escape(TypeName(a.vttype->ElementIfNil()));
            } else {
                s += "any";
            }
            s += doc_tags[Tags::ParamType][1];
            s += doc_tags[Tags::Font][1];
            if (a.optional && (int)i > last_not_optional) {
                s += doc_tags[Tags::ParamDefault][0];
                switch (a.vttype->t) {
                    case V_INT:
                        if (a.flags & NF_BOOL)
                            append(s, a.default_val ? "true" : "false");
                        else
                            append(s, a.default_val);
                        break;
                    case V_FLOAT:
                        append(s, (float)a.default_val);
                        break;
                    default:
                        s += "nil";
                }
                s += doc_tags[Tags::ParamDefault][1];
            }
            s += doc_tags[Tags::Param][1];
        }
        s += doc_tags[Tags::Params][1];
        if (nf->retvals.size()) {
            s += doc_tags[Tags::Returns][0];
            for (auto [i, a] : enumerate(nf->retvals)) {
                s += doc_tags[Tags::RetTypeWrap][0];
                s += doc_tags[Tags::Font][0];
                s += escape(TypeName(a.vttype));
                s += doc_tags[Tags::Font][1];
                s += doc_tags[Tags::RetTypeWrap][1];
                if (i < nf->retvals.size() - 1) s += ", ";
            }
            s += doc_tags[Tags::Returns][1];
        }
        s += cat(doc_tags[Tags::Help][0], escape(nf->def.help), doc_tags[Tags::Help][1], "\n");
        s += doc_tags[Tags::Row][1];
        is_first_row = false;
    }
    s += doc_tags[Tags::Table][1];
    s += doc_tags[Tags::Doc][1];
    return s;
}

void DumpBuiltinDoc(NativeRegistry &nfr, bool group_subsystem) {
    string html_tags[NumTags][2] = {
    /* Doc          */  {"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"
    /*              */   "<html>\n<head>\n<title>lobster builtin function reference</title>\n"
    /*              */   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
    /*              */   "<style type=\"text/css\">"
    /*              */   "table.a, tr.a, td.a {font-size: 10pt;border: 1pt solid #DDDDDD;"
    /*              */   " border-Collapse: collapse; max-width: 88em}</style>\n"
    /*              */   "</head>\n<body><center><table border=0><tr><td>\n<p>"
    /*              */   "lobster builtin functions:"
    /*              */   "(file auto generated by compiler, do not modify)</p></td></tr>", "\n</table></center></body>\n</html>\n"},
    /* Table        */  {"<tr><td><table class=\"a\" border=1 cellspacing=0 cellpadding=4>", "</table></td></tr>\n"},
    /* Row          */  {"<tr class=\"a\" valign=top>", "</tr>\n"},
    /* FirstRow     */  {"<tr class=\"a\" valign=top>", "</tr>\n"},
    /* Subsystem    */  {"<td><h3>", "</h3></td>"},
    /* Name         */  {"<td class=\"a\"><tt><b>", "</b>"},
    /* Params       */  {"(", ")"},
    /* Help         */  {"</tt></td><td class=\"a\">", "</td>\n"},
    /* Font         */  {"<font color=\"#666666\">", "</font>"},
    /* Returns      */  {" -> ", ""},
    /* ParamType    */  {": ", ""},
    /* ParamName    */  {"", ""},
    /* ParamDefault */  {" = ", ""},
    /* Param        */  {"", ""},
    /* RetTypeWrap  */  {"", ""}};
    string s = GetBuiltinDoc(nfr, group_subsystem, html_tags, HTMLEscape);
    WriteFile("builtin_functions_reference.html", false, s, false);
}

void DumpBuiltinDocJson(NativeRegistry &nfr) {
    string json_tags[NumTags][2] = {
    /* Doc          */ {"", ""},
    /* Table        */ {"[", "]"},
    /* Row          */ {",\n{", "}"},
    /* FirstRow     */ {"{", "}"},
    /* Subsystem    */ {"\"subsystem\": \"", "\", "},
    /* Name         */ {"\"funcname\": \"", "\", "},
    /* Params       */ {"\"args\":[", "]"},
    /* Help         */ {", \"doc\": \"", "\""},
    /* Font         */ {"",""},
    /* Returns      */ {", \"returns\": [", "]"},
    /* ParamType    */ {", \"type\": \"", "\""},
    /* ParamName    */ {"\"name\": \"", "\""},
    /* ParamDefault */ {", \"default\": \"", "\""},
    /* Param        */ {"{", "}"},
    /* RetTypeWrap  */ {"\"", "\""}};
    cout << GetBuiltinDoc(nfr, false, json_tags, JSONEscape);
}

}  // namespace lobster
