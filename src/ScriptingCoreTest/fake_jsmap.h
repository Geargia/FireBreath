/**********************************************************\ 
Original Author: Georg Fritzsche

Created:    Dec 22, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Georg Fritzsche, Firebreath development team
\**********************************************************/

#ifndef H_FAKE_JS_MAP
#define H_FAKE_JS_MAP

#include "BrowserObjectAPI.h"

class FakeJsMap : public FB::BrowserObjectAPI
{
    typedef std::vector<std::string> StringVec;
    typedef FB::JSOutMap::value_type OutPair;

    struct GrabKeys {
        std::back_insert_iterator<StringVec> inserter;
        GrabKeys(StringVec& keys) : inserter(std::back_inserter(keys)) {}
        void operator()(const OutPair& p) {
            *inserter++ = p.first;
        }
    };
public:
    FakeJsMap(const FB::JSOutMap& values)
      : FB::BrowserObjectAPI(0), m_values(values)
    {
        std::for_each(m_values.begin(), m_values.end(), GrabKeys(m_names));
    }

    bool HasMethod(std::string) { return false; }
    void SetProperty(int,const FB::variant) {}
    void SetProperty(std::string,const FB::variant) {}
    FB::variant Invoke(std::string,FB::VariantList&) { return FB::variant(); }

    // Methods for enumeration
    virtual void getMemberNames(StringVec &names) 
    {
        names = m_names;
    }
    
    virtual size_t getMemberCount() 
    {
        return m_values.size(); 
    }


    bool HasProperty(std::string s)    
    { 
        return (s == "length"); 
    }
    
    bool HasProperty(int index)        
    { 
        return ((unsigned)index < m_values.size()); 
    }
    
    FB::variant GetProperty(std::string s) 
    { 
        if(s == "length")
            return (int)m_values.size();
        FB::JSOutMap::const_iterator it = m_values.find(s);
        if(it != m_values.end())
            return it->second;
        throw FB::script_error(std::string("no such property '")+s+"'");
    }
    
    FB::variant GetProperty(int index)     
    { 
        if((unsigned)index >= m_values.size()) {
            std::ostringstream ss;
            ss << "index out of range - got " << index << ", size is " << m_values.size();
            throw FB::script_error(ss.str());
        }
        return m_values[m_names[index]];
    }

private:
    FB::JSOutMap m_values;
    StringVec m_names;
};

#endif