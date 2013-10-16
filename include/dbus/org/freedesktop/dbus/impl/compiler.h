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

#ifndef DBUS_ORG_FREEDESKTOP_DBUS_IMPL_COMPILER_H_
#define DBUS_ORG_FREEDESKTOP_DBUS_IMPL_COMPILER_H_

#include <org/freedesktop/dbus/generator.h>

namespace org
{
namespace freedesktop
{
namespace dbus
{
struct Compiler::Private
{
    std::shared_ptr<IntrospectionParser> parser;
    std::shared_ptr<Generator> generator;
    struct Context
    {
        Context()
        {
        }

        void reset()
        {
            root = current = std::shared_ptr<Element>();
        }

        void adjust_up()
        {
            current = current->parent();            
        }

        template<typename T>
        void adjust_down(const T& t)
        {
            if (!root)
            {
                root = std::make_shared<Element>(std::shared_ptr<Element>(), t);
                current = root;
            } else
            {
                auto element = std::make_shared<Element>(current, t);
                current->add_child(element);
                current = element;
            }
        }

        std::shared_ptr<Element> root;
        std::shared_ptr<Element> current;
    } context;

    void on_node(const IntrospectionParser::Node& node)
    {
        context.adjust_down(node);
    }

    void on_node_done()
    {
        context.adjust_up();
    }

    void on_interface(const IntrospectionParser::Interface& interface)
    {
        context.adjust_down(interface);
    }

    void on_interface_done()
    {
        context.adjust_up();
    }

    void on_method(const IntrospectionParser::Method& method)
    {
        context.adjust_down(method);
    }

    void on_method_done()
    {
        context.adjust_up();
    }

    void on_signal(const IntrospectionParser::Signal& signal)
    {
        context.adjust_down(signal);
    }

    void on_signal_done()
    {
        context.adjust_up();
    }

    void on_property(const IntrospectionParser::Property& property)
    {
        context.adjust_down(property);
    }

    void on_argument(const IntrospectionParser::Argument& argument)
    {
        context.adjust_down(argument);
    }

    void on_argument_done()
    {
        context.adjust_up();
    }

    void on_annotation(const IntrospectionParser::Annotation& annotation)
    {
        context.adjust_down(annotation);
    }

    void on_annotation_done()
    {
        context.adjust_up();
    }
};

Compiler::Compiler(
    const std::shared_ptr<IntrospectionParser>& parser,
    const std::shared_ptr<Generator>& generator)
        : d{new Private{parser, generator, Private::Context{}}}
{
    // TODO: Assert on null deps here.
    d->parser->on_node(std::bind(&Private::on_node, d.get(), std::placeholders::_1));
    d->parser->on_node_done(std::bind(&Private::on_node_done, d.get()));
    d->parser->on_interface(std::bind(&Private::on_interface, d.get(), std::placeholders::_1));
    d->parser->on_interface_done(std::bind(&Private::on_interface_done, d.get()));
    d->parser->on_method(std::bind(&Private::on_method, d.get(), std::placeholders::_1));
    d->parser->on_method_done(std::bind(&Private::on_method_done, d.get()));
    d->parser->on_property(std::bind(&Private::on_property, d.get(), std::placeholders::_1));
    d->parser->on_signal(std::bind(&Private::on_signal, d.get(), std::placeholders::_1));
    d->parser->on_signal_done(std::bind(&Private::on_signal_done, d.get()));
    d->parser->on_argument(std::bind(&Private::on_argument, d.get(), std::placeholders::_1));
    d->parser->on_argument_done(std::bind(&Private::on_argument_done, d.get()));
    d->parser->on_annotation(std::bind(&Private::on_annotation, d.get(), std::placeholders::_1));
    d->parser->on_annotation_done(std::bind(&Private::on_annotation_done, d.get()));
}

Compiler::~Compiler()
{
}

bool Compiler::process_introspection_file(const std::string& fn)
{
    d->context.reset();

    if (!d->parser->invoke_for(fn))
        return false;
    if (!d->generator->invoke_for_model(d->context.root))
        return false;

    return true;
}
}
}
}

#endif // DBUS_ORG_FREEDESKTOP_DBUS_IMPL_COMPILER_H_
