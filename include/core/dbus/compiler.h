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

#ifndef CORE_DBUS_COMPILER_H_
#define CORE_DBUS_COMPILER_H_

#include <core/dbus/introspection_parser.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace core
{
namespace dbus
{
class Generator;
struct GeneratorConfiguration;

/**
 * \brief Parses and processes a DBus introspection XML.
 */
class Compiler
{
  public:    
    static int main(int argc, const char** argv);

    class Element
    {
      public:
        enum class Type
        {
            node,
            interface,
            method,
            signal,
            property,
            argument,
            annotation
        };

        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Node& node) 
                : d{parent, Type::node, node}
        {
        }

        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Interface& interface)
                : d{parent, Type::interface, interface}
        {
        }
        
        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Method& method)
                : d{parent, Type::method, method}
        {
        }

        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Signal& signal)
                : d{parent, Type::signal, signal}
        {
        }
        
        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Property& property)
                : d{parent, Type::property, property}
        {
        }

        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Argument& argument)
                : d{parent, Type::argument, argument}
        {
        }

        inline Element(const std::shared_ptr<Element>& parent, const IntrospectionParser::Annotation& annotation)
                : d{parent, Type::annotation, annotation}
        {
        }

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        bool operator<(const Element& rhs) const
        {
            return static_cast<int>(d.type) < static_cast<int>(rhs.d.type);
        }

        inline void apply(std::function<void(const Element&)> before,
                          std::function<void(const Element&)> visitor,
                          std::function<void(const Element&)> after)
        {
            visitor(*this);

            for(auto child : d.children)
            {
                if (before)
                    before(*child);

                child->apply(before, visitor, after);

                if (after)
                    after(*child);
            }
        }

        inline const std::shared_ptr<Element>& parent() const { return d.parent; }        

        inline Type type() const { return d.type; }

        inline void add_child(const std::shared_ptr<Element>& child) { d.children.insert(child); }
        
        inline const IntrospectionParser::Node& node() const { if (d.type != Type::node) throw std::runtime_error("Type mismatch"); return d.payload.node; }
        inline const IntrospectionParser::Interface& interface() const { if (d.type != Type::interface) throw std::runtime_error("Type mismatch"); return d.payload.interface; }
        inline const IntrospectionParser::Method& method() const { if (d.type != Type::method) throw std::runtime_error("Type mismatch"); return d.payload.method; }
        inline const IntrospectionParser::Signal& signal() const { if (d.type != Type::signal) throw std::runtime_error("Type mismatch"); return d.payload.signal; }
        inline const IntrospectionParser::Property& property() const { if (d.type != Type::property) throw std::runtime_error("Type mismatch"); return d.payload.property; }
        inline const IntrospectionParser::Argument& argument() const { if (d.type != Type::argument) throw std::runtime_error("Type mismatch"); return d.payload.argument; }        
        inline const IntrospectionParser::Annotation& annotation() const { if (d.type != Type::annotation) throw std::runtime_error("Type mismatch"); return d.payload.annotation; }

      private:
        struct Private
        {
            struct KeyCompare
            {
                bool operator()(const std::shared_ptr<Element>& lhs,
                                const std::shared_ptr<Element>& rhs) const
                {
                    return *lhs < *rhs;
                }
            };

            template<typename T>
            Private(const std::shared_ptr<Element>& parent, Type type, const T& t)
                : parent(parent),
                  children(KeyCompare()),
                  type(type),
                  payload(t)
            {
            }
            
            std::shared_ptr<Element> parent;
            std::multiset<std::shared_ptr<Element>, KeyCompare> children;
            Type type;
            union Payload
            {
                Payload(const IntrospectionParser::Node& node) : node(node) {}
                Payload(const IntrospectionParser::Interface& interface) : interface(interface) {}
                Payload(const IntrospectionParser::Method& method) : method(method) {}
                Payload(const IntrospectionParser::Signal& signal) : signal(signal) {}
                Payload(const IntrospectionParser::Property& property) : property(property) {}
                Payload(const IntrospectionParser::Argument& argument) : argument(argument) {}
                Payload(const IntrospectionParser::Annotation& annotation) : annotation(annotation) {}
                ~Payload() {}

                IntrospectionParser::Node node;
                IntrospectionParser::Interface interface;
                IntrospectionParser::Method method;
                IntrospectionParser::Signal signal;
                IntrospectionParser::Property property;
                IntrospectionParser::Argument argument;
                IntrospectionParser::Annotation annotation;
            } payload;
        } d;
    };

    /**
     * \brief Result of processing a DBus introspection XML.
     */
    struct Result
    {
        /**
         * \brief Stub elements for usage on clients go here.
         */
        struct
        {
            std::string header_file_path;
            std::string impl_file_path;
        } stub;

        /**
         * \brief Skeleton elements for usage/implementation by services go here.
         */
        struct
        {
            std::string header_file_path;
            std::string impl_file_path;
        } skeleton;
    };

    /**
     * \brief Creates a compiler instance with the given parser and generator.
     * \param[in] parser The parser implementation.
     * \param[in] generator The generator implementation.
     */
    Compiler(const std::shared_ptr<IntrospectionParser>& parser,
             const std::shared_ptr<Generator>& generator);
    Compiler(const Compiler&) = delete;
    ~Compiler();

    bool operator==(const Compiler&) const = delete;
    Compiler& operator=(const Compiler&) = delete;

    /**
     * \brief Parses and processes the DBus introspection XML in the file pointed to by fn.     
     * \brief[in] fn Name of the file containing the DBus introspection XML.
     * \returns An instance of result on success.
     * \throws std::runtime_error in case of errors.
     */
    bool process_introspection_file_with_generator_config(
            const std::string& fn,
            const GeneratorConfiguration& config);

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}

#endif // CORE_DBUS_COMPILER_H_
