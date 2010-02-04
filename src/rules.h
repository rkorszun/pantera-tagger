#ifndef _RULES_H_
#define _RULES_H_

#include <vector>
#include <cstdlib>
#include <functional>

#include <nlpcommon/lexeme.h>

namespace BTagger {

using namespace NLPCommon;

template<class Lexeme>
class Predicate;

template<class Lexeme>
class PredicateTemplate {
public:
    std::vector<const Tagset*> tagsets;

    typedef Lexeme lexeme_type;

    PredicateTemplate(const std::vector<const Tagset*>& tagsets)
            : tagsets(tagsets) { }
    virtual ~PredicateTemplate() { }

    virtual bool predicateMatches(const Predicate<Lexeme>& p,
            std::vector<Lexeme>& text, int index) = 0;
    virtual void findMatchingPredicates(std::vector<Predicate<Lexeme> >& v,
            std::vector<Lexeme>& text, int index) = 0;

    virtual string predicateAsString(const Predicate<Lexeme>& p) = 0;
};

template<class Lexeme>
class Predicate {
public:
    PredicateTemplate<Lexeme>* tpl;
    struct {
        typename Lexeme::tag_type tags[3];
        char chars[4];
        uint8_t categories[3];
        uint8_t values[3];
    } params;

    Predicate() :
        tpl(NULL) {
        std::memset(&params, 0xff, sizeof(params));
    }

    Predicate(PredicateTemplate<Lexeme>* tpl) :
        tpl(tpl) {
        std::memset(&params, 0xff, sizeof(params));
    }

    bool operator==(const Predicate<Lexeme>& r) const {
        return tpl == r.tpl && !std::memcmp(&params, &r.params, sizeof(params));
    }

    bool operator<(const Predicate<Lexeme>& r) const {
        if (tpl == r.tpl)
            return std::memcmp(&params, &r.params, sizeof(params)) < 0;
        return tpl < r.tpl;
    }
};

template<class Lexeme>
std::size_t hash_value(const Predicate<Lexeme>& pred) {
    std::size_t seed = 0;
    boost::hash_combine(seed, pred.tpl);
    boost::hash_range(seed, (char*)&pred.params, (char*)(&pred.params) + sizeof(pred.params));
    return seed;
}

template<class Lexeme>
class Rule {
public:
    Predicate<Lexeme> predicate;
    typename Lexeme::tag_type tag;

    Rule() { }

    Rule(const Predicate<Lexeme>& p, typename Lexeme::tag_type tag) :
        predicate(p), tag(tag) { }

    bool operator==(const Rule<Lexeme>& r) const {
        return r.tag == tag && r.predicate == predicate;
    }

    bool operator<(const Rule<Lexeme>& r) const {
        if (tag < r.tag)
            return true;
        if (tag > r.tag)
            return false;
        return predicate < r.predicate;
    }

    string asString() const {
        PredicateTemplate<Lexeme>* tpl = predicate.tpl;
        const Tagset* tagset = tpl->tagsets[tpl->tagsets.size() - 1];
        return string("(") + predicate.tpl->predicateAsString(predicate)
                + string(") -> ") + tag.asString(tagset);
    }

    operator string() {
        return asString();
    }
};

template<class Lexeme>
std::size_t hash_value(const Rule<Lexeme>& rule) {
    std::size_t seed = 0;
    boost::hash_combine(seed, rule.predicate);
    boost::hash_combine(seed, rule.tag);
    return seed;
}

} // namespace BTagger

#endif
