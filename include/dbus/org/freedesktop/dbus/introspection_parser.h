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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_INTROSPECTION_PARSER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_INTROSPECTION_PARSER_H_

#include <functional>
#include <memory>
#include <string>

namespace org
{
namespace freedesktop
{
namespace dbus
{
class IntrospectionParser
{
  public:
    /**
     * \brief Models an annotation as parsed from the introspection XML.
     */
    struct Annotation
    {
        inline static const char* element_name() { return "annotation"; }
        inline static const char* name_attribute_name() { return "name"; }
        inline static const char* value_attribute_name() { return "value"; }

        std::string name; ///< Name of the annotation, never empty.
        std::string value; ///< Value of the annotation, never empty.
    };
    
    /**
     * \brief Models a node as parsed from the introspection XML.
     */
    struct Node
    {
        inline static const char* element_name() { return "node"; }
        inline static const char* name_attribute_name() { return "name"; }
        std::string name; ///< Name of the node, never empty.
    };

    /**
     * \brief Models an interface as parsed from the introspection XML.
     */
    struct Interface
    {
        inline static const char* element_name() { return "interface"; }
        inline static const char* name_attribute_name() { return "name"; }
        std::string name; ///< Name of the interface, never empty.
    };

    /**
     * \brief Models an argument as parsed from the introspection XML.
     */
    struct Argument
    {
        inline static const char* element_name() { return "arg"; }
        inline static const char* name_attribute_name() { return "name"; }
        inline static const char* type_attribute_name() { return "type"; }
        inline static const char* direction_attribute_name() { return "direction"; }
        /**
         * \brief Direction of the argument.
         */
        enum class Direction
        {
            context, ///< Direction depends on context, i.e., whether it's an arg to a method or a signal.
            in, ///< Argument is passed in.
            out ///< Argument is returned.
        };

        std::string name; ///< Name of the argument, never empty.
        std::string type; ///< Type of the argument, never empty.
        Direction direction; ///< Direction of the argument.
    };

    /**
     * \brief Models a method as parsed from the introspection XML.
     */
    struct Method
    {
        inline static const char* element_name() { return "method"; }
        inline static const char* name_attribute_name() { return "name"; }
        std::string name; ///< Name of the method, never empty.
    };

    /**
     * \brief Models a signal as parsed from the introspection XML.
     */
    struct Signal
    {
        inline static const char* element_name() { return "signal"; }
        inline static const char* name_attribute_name() { return "name"; }
        std::string name; ///< Name of the signal, never empty.
    };
    
    /**
     * \brief Models a property as parsed from the introspection XML.
     */
    struct Property
    {
        inline static const char* element_name() { return "property"; }
        inline static const char* name_attribute_name() { return "name"; }
        inline static const char* type_attribute_name() { return "type"; }
        inline static const char* access_attribute_name() { return "access"; }
        /**
         * \brief Access qualifier.
         */
        enum class Access
        {
            read, ///< Property is readable.
            write, ///< Property is writable.
            read_write ///< Property is both readable and writable.
        };

        std::string name; ///< Name of the property, never empty.
        std::string type; ///< Type of the argument, never empty.
        Access access; ///< Access specification.
    };

    /**
     * \brief Constructs a parser for the given filename.
     * \param [in] filename Name of file to parse introspection XML from.
     * \throws std::runtime_error if parsing fails.     
     */
    IntrospectionParser();
    IntrospectionParser(const IntrospectionParser&) = delete;
    virtual ~IntrospectionParser();

    bool operator==(const IntrospectionParser&) const = delete;
    IntrospectionParser& operator=(const IntrospectionParser&) = delete;

    /**
     * \brief Invokes the parser for the file referred to by filename.
     * \returns True in case of success, false otherwise.
     */
    virtual bool invoke_for(const std::string& filename);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every node.     
     */
    virtual void on_node(const std::function<void(const Node&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every node done.     
     */
    virtual void on_node_done(const std::function<void()>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every interface.
     */
    virtual void on_interface(const std::function<void(const Interface&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every interface done.
     */
    virtual void on_interface_done(const std::function<void()>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every method.
     */
    virtual void on_method(const std::function<void(const Method&)>& f);
    
    /**
     * \brief Registers the supplied lambda function which is then invoked for every method done.
     */
    virtual void on_method_done(const std::function<void()>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every property.
     */
    virtual void on_property(const std::function<void(const Property&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every signal.
     */
    virtual void on_signal(const std::function<void(const Signal&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every signal done.
     */
    virtual void on_signal_done(const std::function<void()>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every argument.
     */
    virtual void on_argument(const std::function<void(const Argument&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every argument done.
     */
    virtual void on_argument_done(const std::function<void()>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every annotation.
     */
    virtual void on_annotation(const std::function<void(const Annotation&)>& f);

    /**
     * \brief Registers the supplied lambda function which is then invoked for every annotation done.
     */
    virtual void on_annotation_done(const std::function<void()>& f);

  private:
    struct Private;
    std::unique_ptr<Private> d;
};
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_INTROSPECTION_PARSER_H_
