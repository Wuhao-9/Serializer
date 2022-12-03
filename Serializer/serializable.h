#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_

#include "DataStream.hpp"

namespace Yazi 
{

namespace serialize
{
    
class DataStream;

#define SERIALIZE(...)                                                                      \
    void serialize(Yazi::serialize::DataStream& stream) const override {                    \
        Yazi::serialize::DataStream::DataType type = Yazi::serialize::DataStream::CUSTOM;   \
        stream.write_type();                                                                \
        stream.write_cst_args(__VA_ARGS__);                                                 \
    }                                                                                       \
                                                                                            \
    bool non_serialize(Yazi::serialize::DataStream& stream) override {                      \
        Yazi::serialize::DataStream::DataType type;                                         \
        if (!stream.read_type(type)) {                                                      \
            return false;                                                                   \
        }                                                                                   \
        stream.read_cst_args(__VA_ARGS__);                                                  \
        return true;                                                                        \
    }                                                                                       \


class serializable {
public:
    virtual void serialize(DataStream& stream) const = 0;
    virtual bool non_serialize(DataStream& stream) = 0;
};



#define STREAM_FOR_WARD()                                                                   \
    Yazi::serialize::serializable& for_ward() {                                             \
        return dynamic_cast<Yazi::serialize::serializable&>(*this);                         \
    }                                                                                       \

}

}


#endif