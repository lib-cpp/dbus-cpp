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
#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_INTROSPECTION_PARSER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_INTROSPECTION_PARSER_H_

#include <org/freedesktop/dbus/introspection_parser.h>

#include <libxml/xmlreader.h>

#include <iostream>
#include <map>

namespace org
{
namespace freedesktop
{
namespace dbus
{
struct IntrospectionParser::Private
{    
    std::function<void(const Node&)> on_node;
    std::function<void()> on_node_done;
    std::function<void(const Interface&)> on_interface;
    std::function<void()> on_interface_done;
    std::function<void(const Method&)> on_method;
    std::function<void()> on_method_done;
    std::function<void(const Property&)> on_property;
    std::function<void(const Signal&)> on_signal;
    std::function<void()> on_signal_done;
    std::function<void(const Argument&)> on_argument;
    std::function<void()> on_argument_done;
    std::function<void(const Annotation&)> on_annotation;
    std::function<void()> on_annotation_done;
    
};

IntrospectionParser::IntrospectionParser() : d(new Private())
{
}

IntrospectionParser::~IntrospectionParser()
{
}

#define NAMED_ENUMERATION_ELEMENT(element) {element, #element}

bool IntrospectionParser::invoke_for(const std::string& filename)
{
    static const char* empty_encoding = nullptr;
    static const int parser_options = XML_PARSE_DTDVALID | XML_PARSE_PEDANTIC;

    auto reader = std::shared_ptr<xmlTextReader>(
        xmlReaderForFile(filename.c_str(),
                         empty_encoding,
                         parser_options),
        [](xmlTextReaderPtr reader)
        {
            if (reader == nullptr)
                return;

            xmlFreeTextReader(reader);
        });

    if (!reader)
        return false;

    const std::map<std::string, std::function<void()>> vtable_done =
    {
        {Node::element_name(), d->on_node_done},
        {Interface::element_name(), d->on_interface_done},
        {Method::element_name(), d->on_method_done},
        {Argument::element_name(), d->on_argument_done},
        {Annotation::element_name(), d->on_annotation_done},
        {Signal::element_name(), d->on_signal_done}
    };

    const std::map<std::string, std::function<void(xmlTextReaderPtr)>> vtable =
    {
        {
            Annotation::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_annotation)
                    return;  
            
                d->on_annotation(
                    Annotation{
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Annotation::name_attribute_name())), 
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Annotation::value_attribute_name()))});
            }
        },
        {
            Node::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_node)
                    return;
            
                d->on_node(Node{reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Node::name_attribute_name()))});
            }
        },
        {
            Interface::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_interface)
                    return;  
            
                d->on_interface(Interface{reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Interface::name_attribute_name()))});
            }
        },
        {
            Argument::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_argument)
                    return;  
            
                static const std::map<std::string, Argument::Direction> direction_lut =
                {
                    {"in", Argument::Direction::in},
                    {"out", Argument::Direction::out}
                };

                Argument::Direction dir = direction_lut.at(
                    reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Argument::direction_attribute_name())));

                d->on_argument(
                    Argument{
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Argument::name_attribute_name())),
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Argument::type_attribute_name())),
                        dir});
            }
        },
        {
            Method::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_method)
                    return;  
            
                d->on_method(
                    Method{
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Method::name_attribute_name()))});
            }
        },
        {
            Signal::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_signal)
                    return;  
            
                d->on_signal(
                    Signal{
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Signal::name_attribute_name()))});
            }
        },
        {
            Property::element_name(),
            [this](xmlTextReaderPtr reader) 
            {
                if (!d->on_property)
                    return;  
                
                static const std::map<std::string, Property::Access> access_lut =
                {
                    {"read", Property::Access::read},
                    {"write", Property::Access::write},
                    {"readwrite", Property::Access::read_write}
                };

                Property::Access a = access_lut.at(
                    reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Property::access_attribute_name())));
                
                d->on_property(
                    Property{
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Property::name_attribute_name())),
                        reinterpret_cast<const char*>(xmlTextReaderGetAttribute(reader, BAD_CAST Property::type_attribute_name())),
                        a});
            }
        }
    };

    enum class NodeType : int
    {
        attribute = 2,
        cdata = 4,
        comment = 8,
        document = 9,
        document_fragment = 11,
        document_type = 10,
        element = 1,
        end_element = 15,
        entity = 6,
        end_entity = 16,
        entity_reference = 5,
        none = 0,
        notation = 12,
        processing_instruction = 7,
        significant_whitespace = 14,
        text = 3,
        whitespace = 13,
        xml_declaration = 17
    };

    static std::map<NodeType, std::string> node_type_lut= 
    {
        NAMED_ENUMERATION_ELEMENT(NodeType::attribute),
        NAMED_ENUMERATION_ELEMENT(NodeType::cdata),
        NAMED_ENUMERATION_ELEMENT(NodeType::comment),
        NAMED_ENUMERATION_ELEMENT(NodeType::document),
        NAMED_ENUMERATION_ELEMENT(NodeType::document_fragment),
        NAMED_ENUMERATION_ELEMENT(NodeType::document_type),
        NAMED_ENUMERATION_ELEMENT(NodeType::element),
        NAMED_ENUMERATION_ELEMENT(NodeType::end_element),
        NAMED_ENUMERATION_ELEMENT(NodeType::entity),
        NAMED_ENUMERATION_ELEMENT(NodeType::end_entity),
        NAMED_ENUMERATION_ELEMENT(NodeType::entity_reference),
        NAMED_ENUMERATION_ELEMENT(NodeType::none),
        NAMED_ENUMERATION_ELEMENT(NodeType::notation),
        NAMED_ENUMERATION_ELEMENT(NodeType::processing_instruction),
        NAMED_ENUMERATION_ELEMENT(NodeType::significant_whitespace),
        NAMED_ENUMERATION_ELEMENT(NodeType::text),
        NAMED_ENUMERATION_ELEMENT(NodeType::whitespace),
        NAMED_ENUMERATION_ELEMENT(NodeType::xml_declaration)
    };

    enum class Status : int
    {
        has_more = 1,
        done = 0,
        error = -1,
    };

    int status = xmlTextReaderRead(reader.get());
    
    if (Status::error == static_cast<Status>(status))
        return false;

    while (Status::has_more == static_cast<Status>(status))
    {
        switch(static_cast<NodeType>(xmlTextReaderNodeType(reader.get())))
        {
            case NodeType::element:
                vtable.at((const char*) xmlTextReaderConstName(reader.get()))(reader.get());
                if (1 == xmlTextReaderIsEmptyElement(reader.get()))
                {
                    if (vtable_done.count((const char*) xmlTextReaderConstName(reader.get())) > 0)
                    {
                        vtable_done.at((const char*) xmlTextReaderConstName(reader.get()))();
                    }
                }
                break;
            case NodeType::end_element:
                if (vtable_done.count((const char*) xmlTextReaderConstName(reader.get())) > 0)
                {
                    vtable_done.at((const char*) xmlTextReaderConstName(reader.get()))();
                }
                break;
            default:
                break;
        }
        status = xmlTextReaderRead(reader.get());
    }
                     
    if (Status::error == static_cast<Status>(status))
        return false;
              
    return Status::done == static_cast<Status>(status);
}

void IntrospectionParser::on_node(const std::function<void(const Node&)>& f)
{
    d->on_node = f;
}

void IntrospectionParser::on_node_done(const std::function<void()>& f)
{
    d->on_node_done = f;
}

void IntrospectionParser::on_interface(const std::function<void(const Interface&)>& f)
{
    d->on_interface = f;
}

void IntrospectionParser::on_interface_done(const std::function<void()>& f)
{
    d->on_interface_done = f;
}

void IntrospectionParser::on_method(const std::function<void(const Method&)>& f)
{
    d->on_method = f;
}

void IntrospectionParser::on_method_done(const std::function<void()>& f)
{
    d->on_method_done = f;
}

void IntrospectionParser::on_property(const std::function<void(const Property&)>& f)
{
    d->on_property = f;
}

void IntrospectionParser::on_signal(const std::function<void(const Signal&)>& f)
{
    d->on_signal = f;
}

void IntrospectionParser::on_signal_done(const std::function<void()>& f)
{
    d->on_signal_done = f;
}

void IntrospectionParser::on_argument(const std::function<void(const Argument&)>& f)
{
    d->on_argument = f;
}

void IntrospectionParser::on_argument_done(const std::function<void()>& f)
{
    d->on_argument_done = f;
}

void IntrospectionParser::on_annotation(const std::function<void(const Annotation&)>& f)
{
    d->on_annotation = f;
}

void IntrospectionParser::on_annotation_done(const std::function<void()>& f)
{
    d->on_annotation_done = f;
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_INTROSPECTION_PARSER_H_
