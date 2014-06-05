/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef CORE_DBUS_TYPES_VARIANT_H_
#define CORE_DBUS_TYPES_VARIANT_H_

#include <core/dbus/codec.h>
#include <core/dbus/message.h>
#include <core/dbus/helper/type_mapper.h>
#include <core/dbus/types/any.h>
#include <core/dbus/types/signature.h>

#include <cstring>

#include <functional>
#include <memory>
#include <stdexcept>

namespace core
{
namespace dbus
{
namespace types
{
class Variant
{
protected:
    typedef std::function<void(dbus::Message::Writer&)> Encoder;
    typedef std::function<void(dbus::Message::Reader&)> Decoder;

    inline Variant(const Encoder& encoder, const types::Signature& signature)
        : encoder(encoder),
          decoder(),
          signature_(signature)
    {
    }

    inline Variant(const Decoder& decoder, const types::Signature& signature)
        : encoder(),
          decoder(decoder),
          signature_(signature)
    {
    }
public:
    template<typename T>
    static inline Variant encode(T t)
    {
        Encoder encoder = [t](dbus::Message::Writer& writer)
        {
            Codec<T>::encode_argument(writer, t);
        };

        return Variant(
                encoder,
                types::Signature(
                        core::dbus::helper::TypeMapper<T>::signature()));
    }

    template<typename T>
    static inline Variant decode(T& t)
    {
        Decoder decoder = [&t](dbus::Message::Reader& reader)
        {
            Codec<T>::decode_argument(reader, t);
        };

        return Variant(
                decoder,
                types::Signature(
                        core::dbus::helper::TypeMapper<T>::signature()));
    }

    inline Variant()
    {
        decoder = [this](dbus::Message::Reader& reader)
        {
            core::dbus::Codec<types::Any>::decode_argument(reader, any);
        };
    }

    inline Variant(const Variant&) = default;

    inline Variant(Variant&& rhs)
        : encoder(std::move(rhs.encoder)),
          decoder(std::move(rhs.decoder)),
          any(std::move(rhs.any)),
          signature_(std::move(rhs.signature_))
    {
    }

    virtual ~Variant() = default;

    inline Variant& operator=(Variant&& rhs)
    {
        encoder = std::move(rhs.encoder);
        decoder = std::move(rhs.decoder);
        signature_ = std::move(rhs.signature_);
        any = std::move(rhs.any);

        return *this;
    }

    virtual void decode(Message::Reader& reader)
    {
        if (!decoder)
            throw std::runtime_error("Variant::decode: Missing a decoder specification.");

        decoder(reader);
    }

    virtual void encode(Message::Writer& writer) const
    {
        if (!encoder)
            throw std::runtime_error("Variant::encode: Missing an encoder specification.");

        encoder(writer);
    }

    virtual const types::Signature& signature() const
    {
        return signature_;
    }

    template<typename T>
    T as() const
    {
        T result;
        any.reader() >> result;

        return result;
    }

protected:
    inline void set_encoder(const Encoder& encoder)
    {
        this->encoder = encoder;
    }

    inline void set_decoder(const Decoder& decoder)
    {
        this->decoder = decoder;
    }

    inline void set_signature(const types::Signature& signature)
    {
        this->signature_ = signature;
    }

private:
    Encoder encoder;
    Decoder decoder;
    types::Any any;
    types::Signature signature_;
};

template<typename T>
class TypedVariant : public Variant
{
public:
    explicit TypedVariant(const T& t = T()) : value(t)
    {
        auto decoder = [this](dbus::Message::Reader& reader)
        {
            Codec<T>::decode_argument(reader, value);
        };

        auto encoder = [this](dbus::Message::Writer& writer)
        {
            Codec<T>::encode_argument(writer, value);
        };

        set_decoder(decoder);
        set_encoder(encoder);
        set_signature(types::Signature{helper::TypeMapper<T>::signature()});
    }

    inline const T& get() const
    {
        return value;
    }

    inline void set(const T& t)
    {
        value = t;
    }

private:
    T value;
};
}
/**
 * @brief Template specialization for variant argument types.
 */
template<>
struct Codec<types::Variant>
{
    inline static void encode_argument(Message::Writer& out, const types::Variant& variant)
    {
        auto vw = out.open_variant(variant.signature());
        {
            variant.encode(vw);
        }
        out.close_variant(std::move(vw));
    }

    inline static void decode_argument(Message::Reader& in, types::Variant& variant)
    {
        auto vr = in.pop_variant();
        variant.decode(vr);
    }
};

template<typename T>
struct Codec<types::TypedVariant<T>>
{
    inline static void encode_argument(Message::Writer& out, const types::TypedVariant<T>& variant)
    {
        Codec<types::Variant>::encode_argument(out, variant);
    }

    inline static void decode_argument(Message::Reader& in, types::TypedVariant<T>& variant)
    {
        Codec<types::Variant>::decode_argument(in, variant);
    }
};
}
}
#endif // CORE_DBUS_TYPES_VARIANT_H_
