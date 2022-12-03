#include "DataStream.hpp"
#include <algorithm>

namespace Yazi 
{

namespace serialize
{

void DataStream::_reserve(size_t len) {
    int size = _m_buf.size();
    int capacity = _m_buf.capacity();
    if (size + len > capacity) {
        while (size + len > capacity) {
            if (capacity == 0) {
                capacity = 1;
            } else {
                capacity *= 2;
            }
        }
        _m_buf.reserve(capacity);
    }
}

void DataStream::_write_base(const void* const pos, size_t len) {
    _reserve(len);
    int size = _m_buf.size();
    _m_buf.resize(size + len);
    std::memcpy(&_m_buf[size], pos, len);
}

void DataStream::write(const bool val) {
    DataType type = DataType::BOOL;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _write_base(static_cast<const void*>(&val), sizeof(bool));
}

void DataStream::write(const char val) {
    DataType type = DataType::CHAR;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _write_base(static_cast<const void*>(&val), sizeof(char));
}

void DataStream::write(const int32_t val) {
    DataType type = DataType::INT32;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _convert_for_write(val);
    _write_base(static_cast<const void*>(&val), sizeof(int32_t));
}

void DataStream::write(const int64_t val) {
    DataType type = DataType::INT64;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _convert_for_write(val);
    _write_base(static_cast<const void*>(&val), sizeof(int64_t));
}

void DataStream::write(const float val) {
    DataType type = DataType::FLOAT;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _convert_for_write(val);
    _write_base(static_cast<const void*>(&val), sizeof(float));
}

void DataStream::write(const double val) {
    DataType type = DataType::DOUBLE;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    _convert_for_write(val);
    _write_base(static_cast<const void*>(&val), sizeof(double));
}

void DataStream::write(const char* val) {
    DataType type = DataType::STRING;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    int len = std::strlen(val);
    write(len);
    _write_base(static_cast<const void*>(val), len);
}

void DataStream::write(const std::string& val) {
    DataType type = DataType::STRING;
    _write_base(static_cast<const void*>(&type), TYPE_LENGTH_BYTE);
    int len = val.size();
    write(len);
    _write_base(static_cast<const char*>(val.data()), len);
}

void DataStream::show() const {
    size_t size = _m_buf.size();
    std::cout << "Data size = " << size << std::endl;
    int idx = 0;
    while (idx < size) {
        switch (*reinterpret_cast<const DataType*>(&_m_buf[idx]))
        {
        case DataType::BOOL :
            if (static_cast<bool>(_m_buf[++idx])) {
                std::cout << std::boolalpha << true << std::noboolalpha;
            } else {
                std::cout << std::boolalpha << false << std::noboolalpha;
            }
            idx += sizeof(bool);
            break;
        case DataType::CHAR :
            std::cout << _m_buf[++idx];
            idx += sizeof(char);
            break;
        case DataType::INT32 :
            std::cout << *reinterpret_cast<const int32_t*>(&_m_buf[++idx]);
            idx += sizeof(int32_t);
            break;
        case DataType::INT64 :
            std::cout << *reinterpret_cast<const int64_t*>(&_m_buf[++idx]);
            idx += sizeof(int64_t);
            break;
        case DataType::FLOAT :
            std::cout << *reinterpret_cast<const float*>(&_m_buf[++idx]);
            idx += sizeof(float);
            break;
        case DataType::DOUBLE :
            std::cout << *reinterpret_cast<const double*>(&_m_buf[++idx]);
            idx += sizeof(double);
            break;
        case DataType::STRING :
            if (static_cast<DataType>(_m_buf[++idx]) == DataType::INT32) {
                int32_t len = *reinterpret_cast<const int32_t*>(&_m_buf[++idx]);
                idx += sizeof(int32_t);
                std::cout << std::string(&_m_buf[idx], len);
                idx += len;
            } else {
                throw std::logic_error("The encoding format of string is incorrect");
            }
            break;

        default:
            throw std::logic_error("Parse to undefined data encoding");
            break;
        }
    }
}

bool DataStream::read(bool& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataType::BOOL) {
        return false;
    }
    val = static_cast<bool>(_m_buf[++_m_pos]);
    _m_pos += sizeof(bool);
    return true;
}

bool DataStream::read(char& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataType::CHAR) {
        return false;
    }
    val = static_cast<char>(_m_buf[++_m_pos]);
    _m_pos += sizeof(char);
    return true;
}

bool DataStream::read(int32_t& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataStream::INT32) {
        return false;
    }
    _convert_for_read<int32_t>();
    val = *reinterpret_cast<const int32_t*>(&_m_buf[++_m_pos]);
    _m_pos += sizeof(int32_t);
    return true;
}

bool DataStream::read(int64_t& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataStream::INT64) {
        return false;
    }
    _convert_for_read<int64_t>();
    val = *reinterpret_cast<const int64_t*>(&_m_buf[++_m_pos]);
    _m_pos += sizeof(int64_t);
    return true;
}

bool DataStream::read(float& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataStream::FLOAT) {
        return false;
    }
    _convert_for_read<float>();
    val = *reinterpret_cast<const float*>(&_m_buf[++_m_pos]);
    _m_pos += sizeof(float);
    return true;
}

bool DataStream::read(double& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataStream::DOUBLE) {
        return false;
    }
    _convert_for_read<double>();
    val = *reinterpret_cast<const double*>(&_m_buf[++_m_pos]);
    _m_pos += sizeof(double);
    return true;
}

bool DataStream::read(std::string& val) {
    if (static_cast<DataType>(_m_buf[_m_pos]) != DataType::STRING) {
        return false;
    }
    if (static_cast<DataType>(_m_buf[++_m_pos]) != DataType::INT32) {
        throw std::logic_error("The encoding format of string is incorrect");
    }
    // !!!!!!
    if (_m_cur_order == BIG_ENDIAN) {
        std::reverse(&_m_buf[_m_pos+1], &_m_buf[_m_pos+1+sizeof(int32_t)]);
    }
    int32_t len = *reinterpret_cast<const int32_t*>(&_m_buf[++_m_pos]);
    _m_pos += sizeof(int32_t);
    val = std::string(&_m_buf[_m_pos], len);
    _m_pos += len;
    return true;
}

bool DataStream::read_type(DataStream::DataType& val) {
    DataStream::DataType type;
    type = static_cast<DataType>(_m_buf[_m_pos]);
    if (type != CUSTOM) {
        return false;
    }
    _m_pos += TYPE_LENGTH_BYTE;
    val = type;
    return true;
}

void DataStream::write_type() {
    DataType cst = CUSTOM;
    _write_base(&cst, TYPE_LENGTH_BYTE);
}

void DataStream::_judge_order() {
    union judge_order {
        int i;
        char c[sizeof(int)];
    } tmp;
    tmp.i = 2;
    if (tmp.c[0] == 0x00000002) {
        return;
    } else {
        _m_cur_order = BIG_ENDIAN;
    }
}


}

}