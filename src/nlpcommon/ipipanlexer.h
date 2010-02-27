/*
 * ipipanlexer.h
 *
 *  Created on: Jan 2, 2010
 *      Author: accek
 */

#ifndef IPIPANLEXER_H_
#define IPIPANLEXER_H_

#include <boost/program_options/detail/convert.hpp>
#include <boost/program_options/detail/utf8_codecvt_facet.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <locale>
#include <nlpcommon/lexer.h>

namespace NLPCommon {

using std::string;

template<class Lexeme = DefaultLexeme>
class IpiPanLexer : public Lexer<Lexeme>
{
private:
    LexerCollector<Lexeme>* collector;
    Lexeme current_lex;
    boost::program_options::detail::utf8_codecvt_facet utf8_facet;

    inline void newLexeme(typename Lexeme::Type lexeme_type) {
        current_lex = Lexeme(lexeme_type);
    }

    void handleChunkStart(const string& type) {
    }

    void handleNoSpace() {
    }

    void handleOrth(const string& orth) {
        newLexeme(Lexeme::SEGMENT);
        current_lex.setOrth(boost::from_8_bit(orth, utf8_facet));
    }

    void handleCtag(const string& ctag, bool disamb) {
        typename Lexeme::tag_type tag =
                Lexeme::tag_type::parseString(collector->getTagset(), ctag);
        current_lex.addAllowedTag(tag);
        if (disamb)
            current_lex.addGoldenTag(tag);
    }

    void handleEndOfTok() {
        collector->collectLexeme(current_lex);
        this->advanceProgress();
    }

public:
    IpiPanLexer(std::istream& stream)
            : Lexer<Lexeme>(stream) { }

    virtual void parseStream(LexerCollector<Lexeme>& collector)
    {
        this->collector = &collector;

        boost::regex parsing_regex = boost::regex(
                "(?:<tok>\\s*<orth>\\s*(.*?)\\s*</orth>)|"
                "(?:<lex\\>([^>]*\\<disamb=[\"']?1[\"']?)?[^>]*>\\s*"
                "<base>\\s*(?:.*?)\\s*</base>\\s*<ctag>\\s*(.*?)\\s*</ctag>"
                "\\s*</lex>)|"
                "(</tok>)");

        enum {
            MATCH_ORTH = 1,
            MATCH_DISAMB = 2,
            MATCH_CTAG = 3,
            MATCH_ETOK = 4
        };

        // This code is heavily based on example from Boost.Regex
        // (http://www.boost.org/doc/libs/1_41_0/libs/regex/doc/html/boost_regex/partial_matches.html)

        char buf[4096];
        const char* next_pos = buf + sizeof(buf);
        while (!this->stream.eof()) {
            std::streamsize leftover = (buf + sizeof(buf)) - next_pos;
            std::streamsize size = next_pos - buf;
            memcpy(buf, next_pos, leftover);
            this->stream.read(buf + leftover, size);
            std::streamsize read = this->stream.gcount();
            next_pos = buf + sizeof(buf);

            boost::cregex_iterator i(buf, buf + read + leftover, parsing_regex,
                    boost::match_default | boost::match_partial);
            boost::cregex_iterator end;

            for (; i != end; ++i) {
                if ((*i)[0].matched == false) {
                    // Partial match, save position and break:
                    next_pos = (*i)[0].first;
                    break;
                }

                if ((*i)[MATCH_ORTH].matched) {
                    handleOrth(i->str(MATCH_ORTH));
                } else if ((*i)[MATCH_CTAG].matched) {
                    handleCtag(i->str(MATCH_CTAG), (*i)[MATCH_DISAMB].matched);
                } else if ((*i)[MATCH_ETOK].matched) {
                    handleEndOfTok();
                }
            }
        }
    }
};

} // namespace NLPCommon

#endif /* IPIPANLEXER_H_ */
