#pragma once

// License: zlib
// Copyright (c) 2019 Juliette Foucaut & Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

/*
API BREAKING CHANGES
====================
- 2020/04/22 - Added tooltipCallback parameter to ImGui::MarkdownConfig
- 2019/02/01 - Changed LinkCallback parameters, see https://github.com/juliettef/imgui_markdown/issues/2
- 2019/02/05 - Added imageCallback parameter to ImGui::MarkdownConfig
- 2019/02/06 - Added useLinkCallback member variable to MarkdownImageData to configure using images as links
*/

/*
imgui_markdown https://github.com/juliettef/imgui_markdown
Markdown for Dear ImGui

A permissively licensed markdown single-header library for https://github.com/ocornut/imgui

Currently requires C++11 or above

imgui_markdown currently supports the following markdown functionality:
 - Wrapped text
 - Headers H1, H2, H3
 - Emphasis
 - Indented text, multi levels
 - Unordered lists and sub-lists
 - Link
 - Image
 - Horizontal rule

Syntax

Wrapping:
Text wraps automatically. To add a new line, use 'Return'.

Headers:
# H1
## H2
### H3

Emphasis:
*emphasis*
_emphasis_
**strong emphasis**
__strong emphasis__

Indents:
On a new line, at the start of the line, add two spaces per indent.
  Indent level 1
    Indent level 2

Unordered lists:
On a new line, at the start of the line, add two spaces, an asterisks and a space.
For nested lists, add two additional spaces in front of the asterisk per list level increment.
  * Unordered List level 1
    * Unordered List level 2

Link:
[link description](https://...)

Image:
![image alt text](image identifier e.g. filename)

Horizontal Rule:
***
___

===============================================================================

// Example use on Windows with links opening in a browser

#include "ImGui.h"                // https://github.com/ocornut/imgui
#include "imgui_markdown.h"       // https://github.com/juliettef/imgui_markdown
#include "IconsFontAwesome5.h"    // https://github.com/juliettef/IconFontCppHeaders

// Following includes for Windows LinkCallback
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Shellapi.h"
#include <string>

void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
inline ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );

static ImFont* H1 = NULL;
static ImFont* H2 = NULL;
static ImFont* H3 = NULL;

static ImGui::MarkdownConfig mdConfig;


void LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    if( !data_.isImage )
    {
        ShellExecuteA( NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL );
    }
}

inline ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    // In your application you would load an image based on data_ input. Here we just use the imgui font texture.
    ImTextureID image = ImGui::GetIO().Fonts->TexID;
    // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };
    ImGui::MarkdownImageData imageData;
    imageData.isValid =         true;
    imageData.useLinkCallback = false;
    imageData.user_texture_id = image;
    imageData.size =            ImVec2( 40.0f, 20.0f );

    // For image resize when available size.x > image width, add
    ImVec2 const contentSize = ImGui::GetContentRegionAvail();
    if( imageData.size.x > contentSize.x )
    {
        float const ratio = imageData.size.y/imageData.size.x;
        imageData.size.x = contentSize.x;
        imageData.size.y = contentSize.x*ratio;
    }

    return imageData;
}

void LoadFonts( float fontSize_ = 12.0f )
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    // Base font
    io.Fonts->AddFontFromFileTTF( "myfont.ttf", fontSize_ );
    // Bold headings H2 and H3
    H2 = io.Fonts->AddFontFromFileTTF( "myfont-bold.ttf", fontSize_ );
    H3 = mdConfig.headingFormats[ 1 ].font;
    // bold heading H1
    float fontSizeH1 = fontSize_ * 1.1f;
    H1 = io.Fonts->AddFontFromFileTTF( "myfont-bold.ttf", fontSizeH1 );
}

void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback( markdownFormatInfo_, start_ );

    switch( markdownFormatInfo_.type )
    {
    // example: change the colour of heading level 2
    case ImGui::MarkdownFormatType::HEADING:
    {
        if( markdownFormatInfo_.level == 2 )
        {
            if( start_ )
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ ImGuiCol_TextDisabled ] );
            }
            else
            {
                ImGui::PopStyleColor();
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

void Markdown( const std::string& markdown_ )
{
    // You can make your own Markdown function with your prefered string container and markdown config.
    // > C++14 can use ImGui::MarkdownConfig mdConfig{ LinkCallback, NULL, ImageCallback, ICON_FA_LINK, { { H1, true }, { H2, true }, { H3, false } }, NULL };
    mdConfig.linkCallback =         LinkCallback;
    mdConfig.tooltipCallback =      NULL;
    mdConfig.imageCallback =        ImageCallback;
    mdConfig.linkIcon =             ICON_FA_LINK;
    mdConfig.headingFormats[0] =    { H1, true };
    mdConfig.headingFormats[1] =    { H2, true };
    mdConfig.headingFormats[2] =    { H3, false };
    mdConfig.userData =             NULL;
    mdConfig.formatCallback =       ExampleMarkdownFormatCallback;
    ImGui::Markdown( markdown_.c_str(), markdown_.length(), mdConfig );
}

void MarkdownExample()
{
    const std::string markdownText = u8R"(
# H1 Header: Text and Links
You can add [links like this one to enkisoftware](https://www.enkisoftware.com/) and lines will wrap well.
You can also insert images ![image alt text](image identifier e.g. filename)
Horizontal rules:
***
___
*Emphasis* and **strong emphasis** change the appearance of the text.
## H2 Header: indented text.
  This text has an indent (two leading spaces).
    This one has two.
### H3 Header: Lists
  * Unordered lists
    * Lists can be indented with two extra spaces.
  * Lists can have [links like this one to Avoyd](https://www.avoyd.com/) and *emphasized text*
)";
    Markdown( markdownText );
}

===============================================================================
*/

#include <stdint.h>

typedef int ImGuiMarkdownFormatFlags;

enum ImGuiMarkdownFormatFlags_
{
    ImGuiMarkdownFormatFlags_None                        = 0,
    ImGuiMarkdownFormatFlags_DiscardExtraNewLines        = 1 << 0,  // (Accurate parsing) Provided markdown will discard all redundant newlines
    ImGuiMarkdownFormatFlags_NoNewLineBeforeHeading      = 1 << 1,  // (Accurate parsing) Provided markdown will not format a newline after the first line if it is a heading
    ImGuiMarkdownFormatFlags_SeparatorDoesNotAdvance     = 1 << 2,  // (Accurate parsing) Provided markdown will not advance to the next line after formatting a separator
    ImGuiMarkdownFormatFlags_GithubStyle                 = ImGuiMarkdownFormatFlags_DiscardExtraNewLines | ImGuiMarkdownFormatFlags_NoNewLineBeforeHeading | ImGuiMarkdownFormatFlags_SeparatorDoesNotAdvance,
    ImGuiMarkdownFormatFlags_CommonMarkAll               = ImGuiMarkdownFormatFlags_DiscardExtraNewLines | ImGuiMarkdownFormatFlags_NoNewLineBeforeHeading | ImGuiMarkdownFormatFlags_SeparatorDoesNotAdvance,
};

namespace ImGui
{
    //-----------------------------------------------------------------------------
    // Basic types
    //-----------------------------------------------------------------------------

    struct Link;
    struct MarkdownConfig;

    struct MarkdownLinkCallbackData                                 // for both links and images
    {
        const char*             text;                               // text between square brackets []
        int                     textLength;
        const char*             link;                               // text between brackets ()
        int                     linkLength;
        void*                   userData;
        bool                    isImage;                            // true if '!' is detected in front of the link syntax
    };

    struct MarkdownTooltipCallbackData                              // for tooltips
    {
        MarkdownLinkCallbackData linkData;
        const char*              linkIcon;
    };

    struct MarkdownImageData
    {
        bool                    isValid = false;                    // if true, will draw the image
        bool                    useLinkCallback = false;            // if true, linkCallback will be called when image is clicked
        ImTextureID             user_texture_id = 0;                // see ImGui::Image
        ImVec2                  size = ImVec2( 100.0f, 100.0f );    // see ImGui::Image
        ImVec2                  uv0 = ImVec2( 0, 0 );               // see ImGui::Image
        ImVec2                  uv1 = ImVec2( 1, 1 );               // see ImGui::Image
        ImVec4                  tint_col = ImVec4( 1, 1, 1, 1 );    // see ImGui::Image
        ImVec4                  border_col = ImVec4( 0, 0, 0, 0 );  // see ImGui::Image
        ImVec4                  bg_col = ImVec4( 0, 0, 0, 0 );      // see ImGui::Image
    };

    enum class MarkdownFormatType
    {
         NORMAL_TEXT,
         HEADING,
         UNORDERED_LIST,
         LINK,
         EMPHASIS,
    };

    struct MarkdownFormatInfo
    {
        MarkdownFormatType      type    = MarkdownFormatType::NORMAL_TEXT;
        int32_t                 level   = 0;                               // Set for headings: 1 for H1, 2 for H2 etc.
        bool                    itemHovered = false;                       // Currently only set for links when mouse hovered, only valid when start_ == false
        const MarkdownConfig*   config  = NULL;
        const char*             text    = NULL;
        int32_t                 textLength = 0;
    };

    typedef void                MarkdownLinkCallback( MarkdownLinkCallbackData data );
    typedef void                MarkdownTooltipCallback( MarkdownTooltipCallbackData data );

    inline void defaultMarkdownTooltipCallback( MarkdownTooltipCallbackData data_ )
    {
        if( data_.linkData.isImage )
        {
            ImGui::SetTooltip( "%.*s", data_.linkData.linkLength, data_.linkData.link );
        }
        else
        {
            ImGui::SetTooltip( "%s Open in browser\n%.*s", data_.linkIcon, data_.linkData.linkLength, data_.linkData.link );
        }
    }

    typedef MarkdownImageData   MarkdownImageCallback( MarkdownLinkCallbackData data );
    typedef void                MarkdownFormalCallback( const MarkdownFormatInfo& markdownFormatInfo_, bool start_ );

    inline void defaultMarkdownFormatCallback( const MarkdownFormatInfo& markdownFormatInfo_, bool start_ );

    struct MarkdownHeadingFormat
    {
        ImFont*                 font;                               // ImGui font
        bool                    separator;                          // if true, an underlined separator is drawn after the header
        #ifdef IMGUI_HAS_TEXTURES // used to detect dynamic font capability: https://github.com/ocornut/imgui/issues/8465#issuecomment-2701570771
        float                   fontSize = 0.0f;                    // Font size if using dynamic fonts
        #endif
    };

    // Configuration struct for Markdown
    // - linkCallback is called when a link is clicked on
    // - linkIcon is a string which encode a "Link" icon, if available in the current font (e.g. linkIcon = ICON_FA_LINK with FontAwesome + IconFontCppHeaders https://github.com/juliettef/IconFontCppHeaders)
    // - headingFormats controls the format of heading H1 to H3, those above H3 use H3 format
    struct MarkdownConfig
    {
        static const int         NUMHEADINGS = 3;

        MarkdownLinkCallback*    linkCallback = NULL;
        MarkdownTooltipCallback* tooltipCallback = NULL;
        MarkdownImageCallback*   imageCallback = NULL;
        const char*              linkIcon = "";                      // icon displayd in link tooltip
        MarkdownHeadingFormat    headingFormats[ NUMHEADINGS ] = { { NULL, true }, { NULL, true }, { NULL, true } };
        void*                    userData = NULL;
        MarkdownFormalCallback*  formatCallback = defaultMarkdownFormatCallback;
        ImGuiMarkdownFormatFlags formatFlags = ImGuiMarkdownFormatFlags_None;  // Configure this to change how Markdown gets formatted. By default imgui_markdown uses psuedo-Markdown for backwards compatibility.
    };

    //-----------------------------------------------------------------------------
    // External interface
    //-----------------------------------------------------------------------------

    inline void Markdown( const char* markdown_, size_t markdownLength_, const MarkdownConfig& mdConfig_ );

    //-----------------------------------------------------------------------------
    // Internals
    //-----------------------------------------------------------------------------

    struct TextRegion;
    struct Line;
    inline void UnderLine( ImColor col_ );
    inline void RenderLine( const char* markdown_, Line& line_, TextRegion& textRegion_, const MarkdownConfig& mdConfig_ );

    struct TextRegion
    {
        TextRegion() : indentX( 0.0f )
        {
        }
        ~TextRegion()
        {
            ResetIndent();
        }

        void RenderTextWrapped( const char* text_, const char* text_end_, bool bIndentToHere_ = false );

        void RenderListTextWrapped( const char* text_, const char* text_end_ )
        {
            ImGui::Bullet();
            ImGui::SameLine();
            RenderTextWrapped( text_, text_end_, true );
        }

        bool RenderLinkText( const char* text_, const char* text_end_, const Link& link_,
            const char* markdown_, const MarkdownConfig& mdConfig_, const char** linkHoverStart_ );

        void RenderLinkTextWrapped( const char* text_, const char* text_end_, const Link& link_,
            const char* markdown_, const MarkdownConfig& mdConfig_, const char** linkHoverStart_, bool bIndentToHere_ = false );

        void ResetIndent()
        {
            if( indentX > 0.0f )
            {
                ImGui::Unindent( indentX );
            }
            indentX = 0.0f;
        }

    private:
        float       indentX;
    };

    // Text that starts after a new line (or at beginning) and ends with a newline (or at end)
    struct Line {
        bool isHeading = false;
        bool isEmphasis = false;
        bool isUnorderedListStart = false;
        bool isLeadingSpace = true;     // spaces at start of line
        int  leadSpaceCount = 0;
        int  headingCount = 0;
        int  emphasisCount = 0;
        int  lineStart = 0;
        int  lineEnd   = 0;
        int  lastRenderPosition = 0;     // lines may get rendered in multiple pieces
    };

    struct TextBlock {                  // subset of line
        int start = 0;
        int stop  = 0;
        int size() const
        {
            return stop - start;
        }
    };

    struct Link {
        enum LinkState {
            NO_LINK,
            HAS_SQUARE_BRACKET_OPEN,
            HAS_SQUARE_BRACKETS,
            HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN,
        };
        LinkState state = NO_LINK;
        TextBlock text;
        TextBlock url;
        bool isImage = false;
        int num_brackets_open = 0;
    };

	struct Emphasis {
		enum EmphasisState {
			NONE,
			LEFT,
			MIDDLE,
			RIGHT,
		};
        EmphasisState state = NONE;
        TextBlock text;
        char sym;
	};

    inline void UnderLine( ImColor col_ )
    {
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        min.y = max.y;
        ImGui::GetWindowDrawList()->AddLine( min, max, col_, 1.0f );
    }

    inline void RenderLine( const char* markdown_, Line& line_, TextRegion& textRegion_, const MarkdownConfig& mdConfig_ )
    {
        // indent
        int indentStart = 0;
        if( line_.isUnorderedListStart )    // ImGui unordered list render always adds one indent
        {
            indentStart = 1;
        }
        for( int j = indentStart; j < line_.leadSpaceCount / 2; ++j )    // add indents
        {
            ImGui::Indent();
        }

        // render
        MarkdownFormatInfo formatInfo;
        formatInfo.config = &mdConfig_;
        int textStart = line_.lastRenderPosition + 1;
        int textSize = line_.lineEnd - textStart;
        if( line_.isUnorderedListStart )    // render unordered list
        {
            formatInfo.type = MarkdownFormatType::UNORDERED_LIST;
            mdConfig_.formatCallback( formatInfo, true );
            const char* text = markdown_ + textStart + 1;
            textRegion_.RenderListTextWrapped( text, text + textSize - 1 );
        }
        else if( line_.isHeading )          // render heading
        {
            formatInfo.level = line_.headingCount;
            formatInfo.type = MarkdownFormatType::HEADING;
            const char* text = markdown_ + textStart + 1;
            formatInfo.text = text;
            formatInfo.textLength = textSize - 1;
            mdConfig_.formatCallback( formatInfo, true );
            textRegion_.RenderTextWrapped( text, text + textSize - 1 );
        }
		else if( line_.isEmphasis )         // render emphasis
		{
			formatInfo.level = line_.emphasisCount;
			formatInfo.type = MarkdownFormatType::EMPHASIS;
			mdConfig_.formatCallback(formatInfo, true);
			const char* text = markdown_ + textStart;
			textRegion_.RenderTextWrapped(text, text + textSize);
		}
        else                                // render a normal paragraph chunk
        {
            formatInfo.type = MarkdownFormatType::NORMAL_TEXT;
            mdConfig_.formatCallback( formatInfo, true );
            const char* text = markdown_ + textStart;
            textRegion_.RenderTextWrapped( text, text + textSize );
        }
        mdConfig_.formatCallback( formatInfo, false );

        // unindent
        for( int j = indentStart; j < line_.leadSpaceCount / 2; ++j )
        {
            ImGui::Unindent();
        }
    }

    // render markdown
    inline void Markdown( const char* markdown_, size_t markdownLength_, const MarkdownConfig& mdConfig_ )
    {
        static const char* s_linkHoverStart = NULL; // we need to preserve status of link hovering between frames
        static ImGuiID     s_linkHoverID    = 0;
        const char* linkHoverStart = NULL;
        ImGuiID linkHoverID = ImGui::GetID("MDLHS");
        if( linkHoverID == s_linkHoverID )
        {
            linkHoverStart = s_linkHoverStart;
        }

        Line        line;
        Line        prevLine;
        Link        link;
        Emphasis    em;
        TextRegion  textRegion;
        int concurrentEmptyNewlines = 0;
        bool appliedExtraNewline = false;

        char c = 0;
        for( int i=0; i < (int)markdownLength_; ++i )
        {
            c = markdown_[i];               // get the character at index
            if( c == 0 ) { break; }         // shouldn't happen but don't go beyond 0.

            // If we're at the beginning of the line, count any spaces
            if( line.isLeadingSpace )
            {
                if ( (mdConfig_.formatFlags & ImGuiMarkdownFormatFlags_DiscardExtraNewLines) ) // Discard LF and CRLF newlines by markdown spec
                {
                    if ( c == '\n' )
                    {
                        concurrentEmptyNewlines++;
                        line.lineStart += 1;
                        continue;
                    }
                    else if ( ( c == '\r' ) && ( (int)markdownLength_ > i + 1 ) && ( markdown_[i + 1] == '\n' ) )
                    {
                        concurrentEmptyNewlines++;
                        line.lineStart += 2;
                        i += 1;
                        continue;
                    }
                }

                if( c == ' ' )
                {
                    ++line.leadSpaceCount;
                    continue;
                }
                else
                {
                    line.isLeadingSpace = false;
                    line.lastRenderPosition = i - 1;
                    if(( c == '*' ) && ( line.leadSpaceCount >= 2 ))
                    {
                        if( ( (int)markdownLength_ > i + 1 ) && ( markdown_[ i + 1 ] == ' ' ) )    // space after '*'
                        {
                            line.isUnorderedListStart = true;
                            ++i;
                            ++line.lastRenderPosition;
                        }
                        // carry on processing as could be emphasis
                    }
                    else if( c == '#' )
                    {
                        line.headingCount++;
                        bool bContinueChecking = true;
                        int j = i;
                        while( ++j < (int)markdownLength_ && bContinueChecking )
                        {
                            c = markdown_[j];
                            switch( c )
                            {
                            case '#':
                                line.headingCount++;
                                break;
                            case ' ':
                                line.lastRenderPosition = j - 1;
                                i = j;
                                line.isHeading = true;
                                bContinueChecking = false;
                                break;
                            default:
                                line.isHeading = false;
                                bContinueChecking = false;
                                break;
                            }
                        }
                        if( line.isHeading )
                        {
                            // reset emphasis status, we do not support emphasis around headers for now
                            em = Emphasis();
                            continue;
                        }
                    }
                }
            }

            if ( (mdConfig_.formatFlags & ImGuiMarkdownFormatFlags_DiscardExtraNewLines) )
            {
                // In markdown spec, 2 or more consecutive newlines gets converted to a single blank
                // line. The first newline is always digested by this parser so we check for 1 or more here.
                if (!appliedExtraNewline && !prevLine.isHeading && concurrentEmptyNewlines >= 1) {
                    ImGui::NewLine();
                    appliedExtraNewline = true;
                }
            }

            // Test to see if we have a link
            switch( link.state )
            {
            case Link::NO_LINK:
                if( c == '[' && !line.isHeading ) // we do not support headings with links for now
                {
                    link.state = Link::HAS_SQUARE_BRACKET_OPEN;
                    link.text.start = i + 1;
                    if( i > 0 && markdown_[i - 1] == '!' )
                    {
                        link.isImage = true;
                    }
                }
                break;
            case Link::HAS_SQUARE_BRACKET_OPEN:
                if( c == ']' )
                {
                    link.state = Link::HAS_SQUARE_BRACKETS;
                    link.text.stop = i;
                }
                break;
            case Link::HAS_SQUARE_BRACKETS:
                if( c == '(' )
                {
                    link.state = Link::HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN;
                    link.url.start = i + 1;
                    link.num_brackets_open = 1;
                }
                break;
            case Link::HAS_SQUARE_BRACKETS_ROUND_BRACKET_OPEN:
                if( c == '(' )
                {
                    ++link.num_brackets_open;
                }
                else if( c == ')' )
                {
                    --link.num_brackets_open;
                }
                if( link.num_brackets_open == 0 )
                {
                    // reset emphasis status, we do not support emphasis around links for now
                    em = Emphasis();
                    // render previous line content
                    line.lineEnd = link.text.start - ( link.isImage ? 2 : 1 );
                    RenderLine( markdown_, line, textRegion, mdConfig_ );
                    line.leadSpaceCount = 0;
                    link.url.stop = i;
                    line.isUnorderedListStart = false;    // the following text shouldn't have bullets
                    ImGui::SameLine( 0.0f, 0.0f );
                    if( link.isImage )   // it's an image, render it.
                    {
                        bool drawnImage = false;
                        bool useLinkCallback = false;
                        if( mdConfig_.imageCallback )
                        {
                            MarkdownImageData imageData = mdConfig_.imageCallback( { markdown_ + link.text.start, link.text.size(), markdown_ + link.url.start, link.url.size(), mdConfig_.userData, true } );
                            useLinkCallback = imageData.useLinkCallback;
                            if( imageData.isValid )
                            {
#if IMGUI_VERSION_NUM < 19185
                                if( imageData.bg_col.w > 0.0f )
                                {
                                    ImVec2 p = ImGui::GetCursorScreenPos();
                                    ImGui::GetWindowDrawList()->AddRectFilled( p, ImVec2( p.x + imageData.size.x, p.y + imageData.size.y ), ImGui::GetColorU32( imageData.bg_col ));
                                }
                                ImGui::Image( imageData.user_texture_id, imageData.size, imageData.uv0, imageData.uv1, imageData.tint_col, imageData.border_col );
#else
                                ImGui::PushStyleColor( ImGuiCol_Border, imageData.border_col );
                                ImGui::ImageWithBg( imageData.user_texture_id, imageData.size, imageData.uv0, imageData.uv1, imageData.bg_col, imageData.tint_col );
                                ImGui::PopStyleColor();
#endif
                                drawnImage = true;
                            }
                        }
                        if( !drawnImage )
                        {
                            ImGui::Text( "( Image %.*s not loaded )", link.url.size(), markdown_ + link.url.start );
                        }
                        if( ImGui::IsItemHovered() )
                        {
                            if( ImGui::IsMouseReleased( 0 ) && mdConfig_.linkCallback && useLinkCallback )
                            {
                                mdConfig_.linkCallback( { markdown_ + link.text.start, link.text.size(), markdown_ + link.url.start, link.url.size(), mdConfig_.userData, true } );
                            }
                            if( link.text.size() > 0 && mdConfig_.tooltipCallback )
                            {
                                mdConfig_.tooltipCallback( { { markdown_ + link.text.start, link.text.size(), markdown_ + link.url.start, link.url.size(), mdConfig_.userData, true }, mdConfig_.linkIcon } );
                            }
                        }
                    }
                    else                 // it's a link, render it.
                    {
                        textRegion.RenderLinkTextWrapped( markdown_ + link.text.start, markdown_ + link.text.start + link.text.size(), link, markdown_, mdConfig_, &linkHoverStart, false );
                    }
                    ImGui::SameLine( 0.0f, 0.0f );
                    // reset the link by reinitializing it
                    link = Link();
                    line.lastRenderPosition = i;
                    break;
                }
            }

            // Test to see if we have emphasis styling
			switch( em.state )
			{
			case Emphasis::NONE:
				if( link.state == Link::NO_LINK && !line.isHeading )
                {
                    int next = i + 1;
                    int prev = i - 1;
					if( ( c == '*' || c == '_' )
                        && ( i == line.lineStart
                            || markdown_[ prev ] == ' '
                            || markdown_[ prev ] == '\t' ) // empasis must be preceded by whitespace or line start
                        && (int)markdownLength_ > next // emphasis must precede non-whitespace
                        && markdown_[ next ] != ' '
                        && markdown_[ next ] != '\n'
                        && markdown_[ next ] != '\t' )
                    {
						em.state = Emphasis::LEFT;
						em.sym = c;
                        em.text.start = i;
						line.emphasisCount = 1;
						continue;
					}
				}
				break;
			case Emphasis::LEFT:
				if( em.sym == c )
                {
					++line.emphasisCount;
					continue;
				}
                else
                {
					em.text.start = i;
					em.state = Emphasis::MIDDLE;
				}
				break;
			case Emphasis::MIDDLE:
				if( em.sym == c )
                {
					em.state = Emphasis::RIGHT;
					em.text.stop = i;
                   // pass through to case Emphasis::RIGHT
				}
                else
                {
                    break;
                }
            #if __cplusplus >= 201703L
                  [[fallthrough]];
            #endif
			case Emphasis::RIGHT:
				if( em.sym == c )
                {
					if( line.emphasisCount < 3 && ( i - em.text.stop + 1 == line.emphasisCount ) )
                    {
                        // render text up to emphasis
                        int lineEnd = em.text.start - line.emphasisCount;
                        if( lineEnd > line.lineStart )
                        {
                            line.lineEnd = lineEnd;
                            RenderLine( markdown_, line, textRegion, mdConfig_ );
						    ImGui::SameLine( 0.0f, 0.0f );
                            line.isUnorderedListStart = false;
                            line.leadSpaceCount = 0;
                        }
						line.isEmphasis = true;
						line.lastRenderPosition = em.text.start - 1;
                        line.lineStart = em.text.start;
					    line.lineEnd = em.text.stop;
					    RenderLine( markdown_, line, textRegion, mdConfig_ );
					    ImGui::SameLine( 0.0f, 0.0f );
					    line.isEmphasis = false;
					    line.lastRenderPosition = i;
					    em = Emphasis();
                    }
                    continue;
				}
                else
                {
                    em.state = Emphasis::NONE;
                    // render text up to here
                    int start = em.text.start - line.emphasisCount;
                    if( start < line.lineStart )
                    {
                        line.lineEnd = line.lineStart;
                        line.lineStart = start;
                        line.lastRenderPosition = start - 1;
                        RenderLine( markdown_, line, textRegion, mdConfig_ );
                        line.lineStart          = line.lineEnd;
                        line.lastRenderPosition = line.lineStart - 1;
                    }
                }
				break;
			}

            // handle end of line (render)
            if( c == '\n' )
            {
                // first check if the line is a horizontal rule
                line.lineEnd = i;
                if( em.state == Emphasis::MIDDLE && line.emphasisCount >=3 &&
                    ( line.lineStart + line.emphasisCount ) == i )
                {
                    ImGui::Separator();
                }
                else
                {
                    // render the line: multiline emphasis requires a complex implementation so not supporting
                    RenderLine( markdown_, line, textRegion, mdConfig_ );
                }

                // reset the line and emphasis state
                prevLine = line;
				line = Line();
                em = Emphasis();

                line.lineStart = i + 1;
                line.lastRenderPosition = i;

                textRegion.ResetIndent();

                // reset the link
                link = Link();

                concurrentEmptyNewlines = 0;
                appliedExtraNewline = false;
            }
        }

        if( em.state == Emphasis::LEFT && line.emphasisCount >= 3 )
        {
            ImGui::Separator();
        }
        else
        {
            // render any remaining text if last char wasn't 0
            if( markdownLength_ && line.lineStart < (int)markdownLength_ && markdown_[ line.lineStart ] != 0 )
            {
                // handle both null terminated and non null terminated strings
                line.lineEnd = (int)markdownLength_;
                if( 0 == markdown_[ line.lineEnd - 1 ] )
                {
                    --line.lineEnd;
                }
                RenderLine( markdown_, line, textRegion, mdConfig_ );
            }
        }

        if( NULL != linkHoverStart || linkHoverID == s_linkHoverID )
        {
            s_linkHoverStart = linkHoverStart;
            s_linkHoverID    = linkHoverID;
        }
    }

    inline bool TextRegion::RenderLinkText( const char* text_, const char* text_end_, const Link& link_,
        const char* markdown_, const MarkdownConfig& mdConfig_, const char** linkHoverStart_ )
    {
        MarkdownFormatInfo formatInfo;
        formatInfo.config = &mdConfig_;
        formatInfo.type = MarkdownFormatType::LINK;
        mdConfig_.formatCallback( formatInfo, true );
        ImGui::PushTextWrapPos( -1.0f );
        ImGui::TextUnformatted( text_, text_end_ );
        ImGui::PopTextWrapPos();

        bool bThisItemHovered = ImGui::IsItemHovered();
        if(bThisItemHovered)
        {
            *linkHoverStart_ = markdown_ + link_.text.start;
        }
        bool bHovered = bThisItemHovered || ( *linkHoverStart_ == ( markdown_ + link_.text.start ) );

        formatInfo.itemHovered = bHovered;
        mdConfig_.formatCallback( formatInfo, false );

        if(bHovered)
        {
            if( ImGui::IsMouseReleased( 0 ) && mdConfig_.linkCallback )
            {
                mdConfig_.linkCallback( { markdown_ + link_.text.start, link_.text.size(), markdown_ + link_.url.start, link_.url.size(), mdConfig_.userData, false } );
            }
            if( mdConfig_.tooltipCallback )
            {
                mdConfig_.tooltipCallback( { { markdown_ + link_.text.start, link_.text.size(), markdown_ + link_.url.start, link_.url.size(), mdConfig_.userData, false }, mdConfig_.linkIcon } );
            }
        }
        return bThisItemHovered;
    }

    // IsCharInsideWord based on ImGui's CalcWordWrapPositionA
    inline bool IsCharInsideWord( char c_ )
    {
        return c_ != ' ' && c_ != '.' && c_ != ',' && c_ != ';' && c_ != '!' && c_ != '?' && c_ != '\"';
    }

    // ImGui::TextWrapped will wrap at the starting position
    // so to work around this we render using our own wrapping for the first line
    inline void TextRegion::RenderTextWrapped( const char* text_, const char* text_end_, bool bIndentToHere_ )
    {
    #if IMGUI_VERSION_NUM >= 19197
        float       fontSize  = ImGui::GetFontSize();
    #else
        float       scale = ImGui::GetIO().FontGlobalScale;
    #endif
        float       widthLeft = GetContentRegionAvail().x;
        const char* endLine = text_;
        if( widthLeft > 0.0f )
        {
        #if IMGUI_VERSION_NUM >= 19197
            endLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthLeft );
        #else
            endLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthLeft );
        #endif
        }

        if( endLine > text_ && endLine < text_end_ )
        {
            if( IsCharInsideWord( *endLine ) )
            {
                // see if we can do a better cut.
                float       widthNextLine = widthLeft + GetCursorScreenPos().x - GetWindowPos().x; // was GetContentRegionMax().x on IMGUI_VERSION_NUM < 19099
            #if IMGUI_VERSION_NUM >= 19197
                const char* endNextLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthNextLine );
            #else
                const char* endNextLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthNextLine );
            #endif
                if( endNextLine == text_end_ || ( endNextLine <= text_end_ && !IsCharInsideWord( *endNextLine ) ) )
                {
                        // can possibly do better if go to next line
                    endLine = text_;
                }
            }
        }
        ImGui::TextUnformatted( text_, endLine );
        if( bIndentToHere_ )
        {
            float indentNeeded = GetContentRegionAvail().x - widthLeft;
            if( indentNeeded )
            {
                ImGui::Indent( indentNeeded );
                indentX += indentNeeded;
            }
        }
        widthLeft = GetContentRegionAvail().x;
        while( endLine < text_end_ )
        {
            text_ = endLine;
            if( *text_ == ' ' ) { ++text_; }    // skip a space at start of line
        #if IMGUI_VERSION_NUM >= 19197
            endLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthLeft );
        #else
            endLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthLeft );
        #endif
            if( text_ == endLine )
            {
                endLine++;
            }
            ImGui::TextUnformatted( text_, endLine );
        }
    }

    inline void TextRegion::RenderLinkTextWrapped( const char* text_, const char* text_end_, const Link& link_,
        const char* markdown_, const MarkdownConfig& mdConfig_, const char** linkHoverStart_, bool bIndentToHere_ )
        {
            #if IMGUI_VERSION_NUM >= 19197
                float       fontSize  = ImGui::GetFontSize();
            #else
                float       scale = ImGui::GetIO().FontGlobalScale;
            #endif
            float       widthLeft = GetContentRegionAvail().x;
            const char* endLine = text_;
            if( widthLeft > 0.0f )
            {
                #if IMGUI_VERSION_NUM >= 19197
                    endLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthLeft );
                #else
                    endLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthLeft );
                #endif
            }

            if( endLine > text_ && endLine < text_end_ )
            {
                if( IsCharInsideWord( *endLine ) )
                {
                    // see if we can do a better cut.
                    float       widthNextLine = widthLeft + GetCursorScreenPos().x - GetWindowPos().x; // was GetContentRegionMax().x on IMGUI_VERSION_NUM < 19099
                    #if IMGUI_VERSION_NUM >= 19197
                        const char* endNextLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthNextLine );
                    #else
                        const char* endNextLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthNextLine );
                    #endif
                    if( endNextLine == text_end_ || ( endNextLine <= text_end_ && !IsCharInsideWord( *endNextLine ) ) )
                    {
                        // can possibly do better if go to next line
                        endLine = text_;
                    }
                }
            }
            bool bHovered = RenderLinkText( text_, endLine, link_, markdown_, mdConfig_, linkHoverStart_ );
            if( bIndentToHere_ )
            {
                float indentNeeded = GetContentRegionAvail().x - widthLeft;
                if( indentNeeded )
                {
                    ImGui::Indent( indentNeeded );
                    indentX += indentNeeded;
                }
            }
            widthLeft = GetContentRegionAvail().x;
            while( endLine < text_end_ )
            {
                text_ = endLine;
                if( *text_ == ' ' ) { ++text_; }    // skip a space at start of line
                #if IMGUI_VERSION_NUM >= 19197
                    endLine = ImGui::GetFont()->CalcWordWrapPosition( fontSize, text_, text_end_, widthLeft );
                #else
                    endLine = ImGui::GetFont()->CalcWordWrapPositionA( scale, text_, text_end_, widthLeft );
                #endif
                if( text_ == endLine )
                {
                    endLine++;
                }
                bool bThisLineHovered = RenderLinkText( text_, endLine, link_, markdown_, mdConfig_, linkHoverStart_ );
                bHovered = bHovered || bThisLineHovered;
            }
            if( !bHovered && *linkHoverStart_ == markdown_ + link_.text.start )
            {
                *linkHoverStart_ = NULL;
            }
        }


    inline void defaultMarkdownFormatCallback( const MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
    {
        switch( markdownFormatInfo_.type )
        {
        case MarkdownFormatType::NORMAL_TEXT:
            break;
		case MarkdownFormatType::EMPHASIS:
        {
            MarkdownHeadingFormat fmt;
            // default styling for emphasis uses last headingFormats - for your own styling
            // implement EMPHASIS in your formatCallback
            if( markdownFormatInfo_.level == 1 )
            {
                // normal emphasis
 			    if( start_ )
			    {
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ ImGuiCol_TextDisabled ] );
			    }
                else
			    {
                    ImGui::PopStyleColor();
			    }
            }
            else
            {
                // strong emphasis
                fmt = markdownFormatInfo_.config->headingFormats[ MarkdownConfig::NUMHEADINGS - 1 ];
			    if( start_ )
			    {
				    if( fmt.font )
				    {
                        #ifdef IMGUI_HAS_TEXTURES // used to detect dynamic font capability: 
					        ImGui::PushFont( fmt.font, 0.0f ); // Change font and keep current size
                        #else
					        ImGui::PushFont( fmt.font );
                        #endif
				    }
			    }
                else
			    {
				    if( fmt.font )
				    {
					    ImGui::PopFont();
				    }
			    }
            }
            break;
        }
        case MarkdownFormatType::HEADING:
        {
            MarkdownHeadingFormat fmt;
            if( markdownFormatInfo_.level > MarkdownConfig::NUMHEADINGS )
            {
                fmt = markdownFormatInfo_.config->headingFormats[ MarkdownConfig::NUMHEADINGS - 1 ];
            }
            else
            {
                fmt = markdownFormatInfo_.config->headingFormats[ markdownFormatInfo_.level - 1 ];
            }
            if (start_)
            {
                if ( 0 == ( markdownFormatInfo_.config->formatFlags & ImGuiMarkdownFormatFlags_NoNewLineBeforeHeading ) )
                {
                    ImGui::NewLine();
                }
                if (fmt.font)
                {
#ifdef IMGUI_HAS_TEXTURES // used to detect dynamic font capability: https://github.com/ocornut/imgui/issues/8465#issuecomment-2701570771
                    ImGui::PushFont(fmt.font, fmt.fontSize > 0.0f ? fmt.fontSize : fmt.font->LegacySize);
#else
                    ImGui::PushFont(fmt.font);
#endif
                }
            }
            else
            {
                if (fmt.separator)
                {
                    // In markdown the separator does not advance the cursor
                    ImVec2 cursor = ImGui::GetCursorPos();
                    ImGui::Separator();
                    if ( (markdownFormatInfo_.config->formatFlags & ImGuiMarkdownFormatFlags_SeparatorDoesNotAdvance) ) {
                        ImGui::SetCursorPos(cursor);
                    }
                }
                if (fmt.font)
                {
                    ImGui::PopFont();
                }
                ImGui::NewLine();
            }
            break;
        }
        case MarkdownFormatType::UNORDERED_LIST:
            break;
        case MarkdownFormatType::LINK:
            if( start_ )
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ ImGuiCol_ButtonHovered ] );
            }
            else
            {
                ImGui::PopStyleColor();
                if( markdownFormatInfo_.itemHovered )
                {
                    ImGui::UnderLine( ImGui::GetStyle().Colors[ ImGuiCol_ButtonHovered ] );
                }
                else
                {
                    ImGui::UnderLine( ImGui::GetStyle().Colors[ ImGuiCol_Button ] );
                }
            }
            break;
        }
    }
}
