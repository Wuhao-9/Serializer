#ifndef DATASTREAM_HPP_
#define DATASTREAM_HPP_

#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>
#include <list>
#include <set>
#include "serializable.h"

namespace Yazi 
{

namespace serialize
{

class DataStream {
public:
    enum DataType : char {  //设置enum底层基于char -> 1byte
        BOOL,
        CHAR,
        INT32,
        INT64,
        FLOAT,
        DOUBLE,
        STRING,
        VECTOR,
        LIST,
        MAP,
        SET,
        CUSTOM
    };

    enum ByteOrder : char {
        LITTLE_ENDIAN,
        BIG_ENDIAN
    };
    
    DataStream() : _m_pos(0) , _m_cur_order(LITTLE_ENDIAN) { _judge_order(); }
    ~DataStream() {}
    void write(const bool val);
    void write(const char val);
    void write(const int32_t val);
    void write(const int64_t val);
    void write(const float val);
    void write(const double val);
    void write(const char* val);
    void write(const std::string& val);
    
    template<typename T, typename Alloc = std::allocator<T>>
    void write(const std::vector<T, Alloc>& val);

    template<typename T, typename Alloc = std::allocator<T>>
    void write(const std::list<T, Alloc>& val);

    template<typename K, typename V, typename Compare = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
    void write(const std::map<K, V, Compare, Alloc>& val);

    template<typename K, typename Compare = std::less<K>, typename Alloc = std::allocator<K>>
    void write(const std::set<K, Compare, Alloc>& val);

    void write_cst_args() {};

    template<typename FST, typename... PACK>
    void write_cst_args(const FST& cur, const PACK&... args) {  // 由于递归调用，影响性能，设为inline
        write(cur);
        write_cst_args(args...);
    }

    void show() const;

    bool read(bool& val);
    bool read(char& val);
    bool read(int32_t& val);
    bool read(int64_t& val);
    bool read(float& val);
    bool read(double& val);
    bool read(std::string& val);

    template<typename T, typename Alloc = std::allocator<T>>
    bool read(std::vector<T, Alloc>& val);

    template<typename T, typename Alloc = std::allocator<T>>
    bool read(std::list<T, Alloc>& val);

    template<typename K, typename V, typename Compare = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
    bool read(std::map<K, V, Compare, Alloc>& val);

    template<typename K, typename Compare = std::less<K>, typename Alloc = std::allocator<K>>
    bool read(std::set<K, Compare, Alloc>& val);

    bool read_cst_args() { return true; }

    bool read_type(DataStream::DataType& val);  // for CUSTOM

    void write_type();  // for CUSTOM

    template<typename FST, typename... PACK>
    bool read_cst_args(FST& cur, PACK&... args) {
        read(cur);
        return read_cst_args(args...);
    }
    

    template<typename T>
    DataStream& operator<<(const T& val);

    template<typename T>
    inline DataStream& operator>>(T& val);

private:
    void _write_base(const void* const pos, size_t len);
    void _reserve(size_t len);
    void _judge_order();

    template<typename T>
    inline void _convert_for_write(T& val);

    template<typename T>
    inline void _convert_for_read();

private:
    std::vector<char> _m_buf;
    size_t _m_pos;
    ByteOrder _m_cur_order;
    const static unsigned TYPE_LENGTH_BYTE = 1; // 默认用1字节表示值的类型

};

template<typename T>
inline void DataStream::_convert_for_write(T& val) {
    if (_m_cur_order == BIG_ENDIAN) {
        char* beg = reinterpret_cast<char*>(const_cast<typename std::remove_const<T>::type*>(&val));
        char* end = beg + sizeof(T);
        std::reverse(beg, end);
    }
}

template<typename T>
inline void DataStream::_convert_for_read() {
    if (_m_cur_order == BIG_ENDIAN) {
        std::reverse(&_m_buf[_m_pos+1], &_m_buf[_m_pos+1+sizeof(T)]);
    }
}

template<typename T>
DataStream& DataStream::operator<<(const T& val) {
    write(val);
    return *this;
}

template<typename T>
DataStream& DataStream::operator>>(T& val) {
    read(val);
    return *this;
}

template<typename T, typename Alloc>
void DataStream::write(const std::vector<T, Alloc>& val) {
    DataType type = DataType::VECTOR;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    int32_t len = val.size();
    write(len);
    for (const auto& item : val) {
        write(item);
    }
}

template<typename T, typename Alloc>
void DataStream::write(const std::list<T, Alloc>& val) {
    DataType type = DataType::LIST;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    int32_t len = val.size();
    write(len);
    for (const auto& item : val) {
        const T* const cur_item_addr = &item;
        _write_base(cur_item_addr, sizeof(T));
    }
}

template<typename K, typename V, typename Compare, typename Alloc>
void DataStream::write(const std::map<K, V, Compare, Alloc>& val) {
        DataType type = DataType::MAP;
        _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
        int32_t len = val.size();
        write(len);
        for (const auto& item : val) {
            _write_base(&item, sizeof(item));
        }
}

template<typename K, typename Compare, typename Alloc>
void DataStream::write(const std::set<K, Compare, Alloc>& val) {
    DataType type = DataType::SET;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    int32_t len = val.size();
    write(len);
    for (const auto& item : val) {
        write(item);
    }
}

template<typename T, typename Alloc>
bool DataStream::read(std::vector<T, Alloc>& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != VECTOR) {
        return false;
    }
    _m_pos += sizeof(DataType);
    int32_t len;
    read(len);
    for (int i = 0; i < len; ++i) {
        T tmp_t;
        read(tmp_t);
        val.push_back(std::move(tmp_t));
    }
    return true;
}

template<typename T, typename Alloc>
bool DataStream::read(std::list<T, Alloc>& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != LIST) {
        return false;
    }
    _m_pos += sizeof(DataType);
    int32_t len;
    read(len);
    for (int i = 0; i < len; ++i) {
        const T target = *reinterpret_cast<const T*>(&_m_buf[_m_pos]);
        val.push_back(std::move(target));
        _m_pos += sizeof(T);
    }
    return true;
}

template<typename K, typename V, typename Compare, typename Alloc>
bool DataStream::read(std::map<K, V, Compare, Alloc>& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != MAP) {
        return false;
    }
    _m_pos += sizeof(DataType);
    int32_t len;
    read(len);
    for (int i = 0; i < len; ++i) {
        const std::pair<K, V> tmp_pair = *reinterpret_cast<std::pair<K, V>*>(&_m_buf[_m_pos]);
        _m_pos += sizeof(std::pair<K, V>);
        val.insert(std::move(tmp_pair));
    }
    return true;
}

template<typename K, typename Compare, typename Alloc>
bool DataStream::read(std::set<K, Compare, Alloc>& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != SET) {
        return false;
    }
    _m_pos += sizeof(DataType);
    int32_t len;
    read(len);
    for (int i = 0; i < len; ++i) {
        K tmp_k;
        read(tmp_k);
        val.insert(std::move(tmp_k));
    }
    return true;
}

template<>
inline DataStream& DataStream::operator<< <Yazi::serialize::serializable> (const serializable& cst) {
    cst.serialize(*this);
    return *this;
}

template<>
inline DataStream& DataStream::operator>> <Yazi::serialize::serializable>(serializable& cst) {
    cst.non_serialize(*this);
    return *this;
}

}

}
#endif