/*
 * pos.h
 *
 *  Created on: Jan 2, 2010
 *      Author: accek
 */

#ifndef POS_H_
#define POS_H_

#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <nlpcommon/util.h>


namespace NLPCommon {

using std::string;
using std::vector;
using std::map;

class PartOfSpeech {
private:
    string _name;
    vector<const Category*> _categories;
    vector<bool> _requiredCategoryFlags;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & _name;
        ar & _categories;
        ar & _requiredCategoryFlags;
    }

public:
    PartOfSpeech(const string& name) : _name(name) { }

    const string& getName() const { return _name; }
    wstring getWName() const { return ascii_to_wstring(getName()); }

    const vector<const Category*>& getCategories(void) const {
        return _categories;
    }

    bool isRequiredCategory(unsigned int index) const {
        return _requiredCategoryFlags[index];
    }

    bool hasCategory(const Category* category) const {
        vector<const Category*>::const_iterator i =
                std::find(_categories.begin(), _categories.end(), category);
        return i != _categories.end();
    }

    bool isRequiredCategory(const Category* category) const {
        vector<const Category*>::const_iterator i =
                std::find(_categories.begin(), _categories.end(), category);
        if (i == _categories.end())
            return false;
        return _requiredCategoryFlags[i - _categories.begin()];
    }

    void addCategory(const Category* category, bool required) {
        _categories.push_back(category);
        _requiredCategoryFlags.push_back(required);
    }

};

} // namespace NLPCommon

#endif /* POS_H_ */
