#pragma once

#include <vector>
#include <optional>
#include <set>
#include <string>
#include <filesystem>
#include <fstream>
#include <unordered_map>

template <typename Key, typename Value>
struct KV {
    Key key;
    Value value;
};

template <typename Base, typename Joined>
struct JoinResult {
    Base base;
    std::optional<Joined> joined;

    bool operator==(const JoinResult&) const = default;
};

class AdapterBase {};

template<typename Joined>
class JoinKV : public AdapterBase{
    public:
    JoinKV(Joined _joined) : joined(_joined) {}

    template<typename Base>
    class JoinView{
        public:
        JoinView(Base _base, Joined _joined) : begin_it(Iterator(_base.begin(), _joined)),
        end_it(Iterator(_base.end(), _joined)),base(_base), joined(_joined) {}

        using value_type=JoinResult<decltype(std::declval<typename Base::value_type>().value)
        , decltype(std::declval<typename Joined::value_type>().value)>;

        class Iterator{
            public:
            Iterator(Base::iterator _iter, Joined _joined) : iter(_iter), joined(_joined),
            jiter(joined.begin()), jend(joined.end()) {}

            bool operator!=(Iterator other){
                return iter!=other.iter || jiter!=other.jiter;
            }

            value_type operator*(){
                auto key=(*iter).key;
                for(; jiter!=jend; ++jiter){
                    if(key==(*jiter).key){
                        auto val = (*jiter).value;
                        ++jiter;
                        while (jiter != jend && (*jiter).key!=key) {
                            ++jiter;
                        }
                        if (jiter != jend) {
                            --jiter;
                        }
                        return JoinResult<decltype((*iter).value), decltype(val)>
                        ((*iter).value, val);
                    }
                }
                return JoinResult<decltype((*iter).value), decltype(std::declval<typename Joined::value_type>().value)>
                ((*iter).value, std::nullopt);
            }

            Iterator& operator++(){
                if (jiter == jend) {
                    ++iter;
                    jiter = joined.begin();
                }
                else
                    ++jiter;
                return *this;
            }

            private:
            Base::iterator iter;
            Joined joined;
            Joined::iterator jiter;
            Joined::iterator jend;
        };

        using iterator = Iterator;

        iterator begin(){
            return begin_it;
        }

        iterator end(){
            return end_it;
        }
        
        private:
        iterator begin_it;
        iterator end_it;
        Base base;
        Joined joined;
    };
    
    template<typename Base>
    auto operator()(Base base){
        return JoinView<Base>(base, joined);
    }

    private:
    Joined joined;
};

template<typename Joined, typename Func1, typename Func2>
class JoinComparators : public AdapterBase{
    public:
    JoinComparators(Joined _joined, Func1 func1, Func2 func2) : joined(_joined), f1(func1), f2(func2) {}

    template<typename Base>
    class JoinView{
        public:
        JoinView(Base _base, Joined _joined, Func1 func1, Func2 func2) : 
        begin_it(Iterator(_base.begin(), _joined, func1,func2)),
        end_it(Iterator(_base.end(), _joined,func1,func2)),base(_base), joined(_joined) {}

        using value_type=JoinResult<typename Base::value_type, typename Joined::value_type>;

        class Iterator{
            public:
            Iterator(Base::iterator _iter, Joined _joined, Func1 func1, Func2 func2):
            iter(_iter), joined(_joined),jiter(joined.begin()), jend(joined.end()),f1(func1),f2(func2) {}

            bool operator!=(Iterator other){
                return iter!=other.iter || jiter!=other.jiter;
            }

            value_type operator*(){
                auto key=f1(*iter);
                for(; jiter!=jend; ++jiter){
                    if(key==f2(*jiter)){
                        auto val = *jiter;
                        ++jiter;
                        while (jiter != jend && f2(*jiter)!=key) {
                            ++jiter;
                        }
                        if (jiter != jend) {
                            --jiter;
                        }
                        return JoinResult<typename Base::value_type, typename Joined::value_type>(*iter, val);
                    }
                }
                return JoinResult<typename Base::value_type, typename Joined::value_type>(*iter, std::nullopt);
            }

            Iterator& operator++(){
                if (jiter == jend) {
                    ++iter;
                    jiter = joined.begin();
                }
                else
                    ++jiter;
                return *this;
            }

            private:
            Base::iterator iter;
            Joined joined;
            Joined::iterator jiter;
            Joined::iterator jend;
            Func1 f1;
            Func2 f2;
        };

        using iterator = Iterator;

        iterator begin(){
            return begin_it;
        }

        iterator end(){
            return end_it;
        }
        
        private:
        iterator begin_it;
        iterator end_it;
        Base base;
        Joined joined;
    };
    
    template<typename Base>
    auto operator()(Base base){
        return JoinView<Base>(base, joined, f1, f2);
    }

    private:
    Joined joined;
    Func1 f1;
    Func2 f2;
};

template<typename Joined>
auto Join(Joined j) {
    return JoinKV<Joined>(j);
}

template<typename Joined, typename Func1, typename Func2>
auto Join(Joined j, Func1 f1, Func2 f2) {
    return JoinComparators<Joined, Func1, Func2>(j, f1, f2);
}

class Dir : public AdapterBase {
public:
    Dir(std::string _path, bool _recursive) : begin_it(Iterator(_path)), end_it() {
        if(_recursive){
            begin_it=Iterator(_path,true);
            end_it=Iterator(true);
        }
    }

    class Iterator {
    public:
        Iterator() : dir(std::filesystem::directory_iterator()), recursive(false){}
        Iterator(bool _recursive) : rec(std::filesystem::recursive_directory_iterator()), recursive(true){}

        Iterator(std::string path) : dir(path), recursive(false){}
        Iterator(std::string path, bool _recursive) : rec(path), recursive(true){}

        Iterator operator++() {
            if (recursive) {
                ++rec;
                return *this;
            }
            ++dir;
            return *this;
        }

        bool operator!=(Iterator other) {
            if (recursive)
                return rec != other.rec;
            return dir != other.dir;
        }

        std::filesystem::directory_entry operator*() {
            if (recursive)
                return *rec;
            return *dir;
        }

    private:
        std::filesystem::directory_iterator dir;
        std::filesystem::recursive_directory_iterator rec;
        bool recursive;
    };

    using value_type = std::filesystem::path;
    using iterator = Iterator;

    iterator begin() const {
        return begin_it;
    }

    iterator end() const {
        return end_it;
    }

private:
    iterator begin_it;
    iterator end_it;
};

class OpenFiles : public AdapterBase {
    public:
    class OpenFilesView{
        public:
        OpenFilesView(Dir::iterator _begin, Dir::iterator _end) : begin_it(_begin,_end), end_it(_end,_end){}

        class Iterator{
            public:
            Iterator(Dir::iterator _b, Dir::iterator _e) : _begin(_b), _end(_e) {}

            std::fstream operator*(){
                return std::fstream((*_begin).path());
            }

            bool operator!=(Iterator other){
                return _begin!=other._begin;
            }

            Iterator& operator++(){
                ++_begin;
                return *this;
            }

            private:
            Dir::iterator _begin;
            Dir::iterator _end;
        };

        using value_type=std::fstream;
        using iterator=Iterator;

        iterator begin(){
            return begin_it;
        }

        iterator end(){
            return end_it;
        }

        private:
        iterator begin_it;
        iterator end_it;
    };

    template<typename T>
    OpenFilesView operator()(Dir dir){
        return OpenFilesView(dir.begin(), dir.end());
    }
};

class Split : public AdapterBase {
public:
    Split(std::string _delimiters) : delimiters(_delimiters) {}

    template<typename Range>
    class SplitView : public AdapterBase {
    public:
        using value_type = std::string;

        SplitView(Range::iterator range_begin, Range::iterator range_end, std::string _delimiters) :
            begin_it(Iterator(range_begin, range_end, _delimiters)), end_it(Iterator(range_end, range_end, _delimiters)) {
        }

        class Iterator {
        public:
            Iterator(Range::iterator _r_begin, Range::iterator _r_end, std::string _delimiters) :
                r_begin(_r_begin), r_end(_r_end) {
                for (char c : _delimiters) {
                    delimiters.insert(c);
                }
                if (r_begin != r_end) {
                    char c;
                    while ((*r_begin).get(c)) {
                        if (delimiters.contains(c)) {
                            break;
                        }
                        cur.push_back(c);
                    }
                }
            }

            Iterator& operator++() {
                if (r_begin != r_end) {
                    char c;
                    do {
                        while ((*r_begin).get(c)) {
                            if (delimiters.contains(c)) {
                                break;
                            }
                            cur.push_back(c);
                        }
                        if (!delimiters.contains(c)) {
                            ++r_begin;
                            break;
                        }
                    } while (!delimiters.contains(c));
                }
                return *this;
            }

            std::string operator*() {
                std::string tmp = cur;
                cur = "";
                return tmp;
            }

            bool operator!=(Iterator other) {
                return (cur != other.cur) || (r_begin != other.r_begin);
            }

        private:
            std::string cur = "";
            typename Range::iterator r_begin;
            typename Range::iterator r_end;
            std::set<char> delimiters;
        };

        using iterator = Iterator;

        iterator begin() const {
            return begin_it;
        }

        iterator end() const {
            return end_it;
        }

    private:
        iterator begin_it;
        iterator end_it;
    };

    template<typename Range>
    SplitView<Range> operator()(Range range) {
        return SplitView<Range>(range.begin(), range.end(), delimiters);
    }

private:
    std::string delimiters;
};

template<typename Container>
class AsDataFlow : public AdapterBase {
public:
    using value_type = Container::value_type;
    using iterator = Container::iterator;
    AsDataFlow(Container& data) : begin_iterator(data.begin()), end_iterator(data.end()) {}
    iterator begin() const {
        return begin_iterator;
    }
    iterator end() const {
        return end_iterator;
    }

private:
    iterator begin_iterator;
    iterator end_iterator;
};

template<typename Func>
class Transform : public AdapterBase{
    public:
    Transform(Func func) : f(func) {}

    template<typename Range>
    class TransformView{
        public:
        TransformView(Range::iterator r_begin, Range::iterator r_end, Func func) : 
        begin_it(Iterator(r_begin, func)), end_it(Iterator(r_end, func)) {}

        class Iterator{
            public:
            Iterator(Range::iterator iter, Func func): it(iter), f(func){}

            bool operator!=(Iterator other){
                return it!=other.it;
            }

            Iterator& operator++(){
                ++it;
                return *this;
            }

            auto operator*(){
                return f(*it);
            }

            private:
            typename Range::iterator it;
            Func f;
        };

        using iterator=Iterator;
        using value_type = decltype(std::declval<Func>()(std::declval<typename Range::value_type>()));

        iterator begin(){
            return begin_it;
        }

        iterator end(){
            return end_it;
        }

        private:
        iterator begin_it;
        iterator end_it;
    };

    template<typename Range>
    TransformView<Range> operator()(Range range){
        return TransformView<Range>(range.begin(), range.end(), f);
    }

    private:
    Func f;
};


template<typename Func>
class Filter : public AdapterBase {
public:
    Filter(Func predicate) : f(predicate) {}

    template<typename Flow>
    class FilterView : public AdapterBase {
    public:
        FilterView(Func predicate, Flow::iterator begin_it, Flow::iterator end_it) : f(predicate),
            begin_iterator(begin_it, end_it, predicate), end_iterator(end_it, end_it, predicate) {
        }

        class Iterator {
        public:
            Iterator(Flow::iterator flow_begin, Flow::iterator flow_end, Func func) :
                it1(flow_begin), it2(flow_end), f(func) {
                Skip();
            }

            Iterator& operator++() {
                ++it1;
                Skip();
                return *this;
            }

            Flow::value_type& operator*() {
                return *it1;
            }

            bool operator!=(Iterator& other) {
                return it1 != other.it1;
            }

        private:
            Flow::iterator it1;
            Flow::iterator it2;
            Flow::iterator next;
            Func f;

            void Skip() {
                while (it1 != it2 && !f(*it1)) {
                    ++it1;
                }
            }
        };

        using value_type = typename Flow::value_type;
        using iterator = Iterator;

        iterator begin() const {
            return begin_iterator;
        }
        iterator end() const {
            return end_iterator;
        }

    private:
        iterator begin_iterator;
        iterator end_iterator;
        Func f;
    };

    template<typename Flow>
    FilterView<Flow> operator()(Flow data) {
        return FilterView<Flow>(f, data.begin(), data.end());
    }

private:
    Func f;
};

class AsVector : public AdapterBase {
public:
    template<typename Flow>
    std::vector<typename Flow::value_type> operator() (Flow& data) {
        std::vector<typename Flow::value_type> vec;
        for (typename Flow::value_type i : data) {
            vec.push_back(i);
        }
        return vec;
    }
};

class DropNullopt : public AdapterBase {
public:
    template<typename T>
    auto operator()(T range) {
        return Filter([](const auto& value) {return value.has_value();})(range);
    }
};

class Write : public AdapterBase {
public:
    Write(std::ostream& stream, char c = '\n') : s(stream), delimiter(c) {}

    template<typename T>
    bool operator()(T range) {
        try {
            for (auto i : range) {
                s << i << delimiter;
            }
            return true;
        }
        catch (...) {
            return false;
        }
    }

private:
    std::ostream& s;
    char delimiter;
};

class Out : public Write {
    Out(std::ostream& stream) : Write(stream) {}
};

class SplitExpected : public AdapterBase{
    public:
    template<typename Range>
    std::pair<std::vector<std::string>, std::vector<typename Range::value_type>> operator()(Range range){
        std::pair<std::vector<std::string>, std::vector<typename Range::value_type>> exp;
        for(auto i : range){
            if(i.has_value()){
                exp.second.push_back(i.value());
            }
            else{
                exp.first.push_back(i.error());
            }
        }
        return exp;
    }
};

template<typename Value, typename Func1, typename Func2>
class AggregateByKey : public AdapterBase {
    public:
    AggregateByKey(Value val, Func1 func1, Func2 func2) : value(val), f1(func1), f2(func2) {}
    template<typename Range>
    auto operator()(Range range){
        std::unordered_map<decltype(f2(std::declval<typename Range::value_type>())), Value> umap;
        for(auto& i : range){
            f1(i,umap[f2(i)]);
        }
        return umap;
    }
    
    private:
    Value value;
    Func1 f1;
    Func2 f2;
};

template <typename T>
concept Adapter = std::derived_from<T, AdapterBase>;

template <typename Range, typename A> requires Adapter<A>
auto operator|(Range range, A adapter) {
    return adapter(range);
}
