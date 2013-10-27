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

#include <org/freedesktop/dbus/generator.h>

#include <org/freedesktop/dbus/argument_type.h>
#include <org/freedesktop/dbus/generator_configuration.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <chrono>
#include <initializer_list>
#include <iomanip>
#include <queue>
#include <sstream>
#include <string>

#include <ctime>

typedef boost::filesystem::path Path;
typedef boost::filesystem::fstream File;

namespace org
{
namespace freedesktop
{
namespace dbus
{
namespace detail
{
struct TypeMangler
{
    struct Comma
    {
        enum class State
        {
            not_required,
            not_armed,
            armed
        };

        mutable State state = State::not_required;

        friend std::ostream& operator<<(std::ostream& out, const Comma& comma)
        {
            switch(comma.state)
            {
            case State::not_required: comma.state = State::not_armed; break;
            case State::not_armed: comma.state = State::armed; break;
            case State::armed: out << ","; comma.state = State::not_armed; break;
            }

            return out;
        }
    };

    struct State
    {
        struct Element
        {
            org::freedesktop::dbus::ArgumentType argument;
            std::shared_ptr<Element> parent;
            std::vector<std::shared_ptr<Element>> children;
        };

        State() : root(new Element()), current(root)
        {
        }

        bool update_for_type(ArgumentType arg)
        {
            if (ArgumentType::invalid == arg)
                return false;

            is_void = false;

            static const std::vector<std::shared_ptr<Element>> no_children;
            std::shared_ptr<Element> new_element(new Element{arg, current, no_children});
            current->children.push_back(new_element);

            return true;
        }

        bool update_for_array_start()
        {
            current = current->children.back();
            return true;
        }

        bool update_for_struct_start()
        {
            current = current->children.back();
            return true;
        }

        bool update_for_struct_end()
        {
            current = current->parent;
            return true;
        }

        bool update_for_dict_entry_start()
        {
            current = current->children.back();
            return true;
        }

        bool update_for_dict_entry_end()
        {
            current = current->parent;
            return true;
        }
        bool is_void = true;
        std::shared_ptr<Element> root;
        std::shared_ptr<Element> current;
    };

    bool update_from_signature(const std::string& s)
    {
        return update_from_signature(s.begin(), s.end());
    }

    bool update_from_signature(std::string::const_iterator it, std::string::const_iterator itE)
    {
        while (it != itE)
        {
            if (!update(*it))
                return false;

            ++it;
        }

        return true;
    }

    bool update(char c)
    {
        bool result = false;

        switch (static_cast<int>(c))
        {
        case DBUS_STRUCT_BEGIN_CHAR:
            result = state.update_for_type(ArgumentType::structure);
            result = state.update_for_struct_start();
            break;
        case DBUS_STRUCT_END_CHAR:
            result = state.update_for_struct_end();
            break;
        case DBUS_DICT_ENTRY_BEGIN_CHAR:
            result = state.update_for_type(ArgumentType::dictionary_entry);
            result = state.update_for_dict_entry_start();
            break;
        case DBUS_DICT_ENTRY_END_CHAR:
            result = state.update_for_dict_entry_end();
            break;
        case DBUS_TYPE_ARRAY:
            result = state.update_for_type(static_cast<org::freedesktop::dbus::ArgumentType>(c));
            result = state.update_for_array_start();
            break;
        default:
            result = state.update_for_type(static_cast<org::freedesktop::dbus::ArgumentType>(c));
            break;
        }

        return result;
    }

    bool is_void() const
    {
        return state.is_void;
    }

    std::string print(const std::shared_ptr<State::Element>& element, const Comma& comma)
    {
        std::stringstream ss;

        switch (element->argument)
        {
        case ArgumentType::invalid:
            if (element->children.size() > 1)
                ss << comma << "std::tuple<";

        {
            Comma scoped_comma;
            for (auto child : element->children)
                ss << print(child, scoped_comma);
        }

            if (element->children.size() > 1)
                ss << ">";
            ss << comma;
            break;
        case ArgumentType::byte:
            ss << comma << "std::uint8_t" << comma;
            break;
        case ArgumentType::boolean:
            ss << comma << "bool" << comma;
            break;
        case ArgumentType::int16:
            ss << comma << "std::int16_t" << comma;
            break;
        case ArgumentType::uint16:
            ss << comma << "std::uint16_t" << comma;
            break;
        case ArgumentType::int32:
            ss << comma << "std::int32_t" << comma;
            break;
        case ArgumentType::uint32:
            ss << comma << "std::uint32_t" << comma;
            break;
        case ArgumentType::int64:
            ss << comma << "std::int64_t" << comma;
            break;
        case ArgumentType::uint64:
            ss << comma << "std::uint64_t" << comma;
            break;
        case ArgumentType::floating_point:
            ss << comma << "double" << comma;
            break;
        case ArgumentType::string:
            ss << comma << "std::string" << comma;
            break;
        case ArgumentType::object_path:
            ss << comma << "org::freedesktop::dbus::types::ObjectPath" << comma;
            break;
        case ArgumentType::signature:
            ss << comma << "org::freedesktop::dbus::types::Signature" << comma;
            break;
        case ArgumentType::unix_fd:
            ss << comma << "org::freedesktop::dbus::types::UnixFd" << comma;
            break;
        case ArgumentType::variant:
            ss << comma << "org::freedesktop::dbus::types::Variant<>" << comma;
            break;
        case ArgumentType::array:
            ss << comma << "std::vector<";
        {
            Comma scoped_comma;
            for (auto child : element->children)
                ss << print(child, scoped_comma);
        }
            ss << ">" << comma;
            break;
        case ArgumentType::structure:
            ss << comma << "std::tuple<";
        {
            Comma scoped_comma;
            for (auto child : element->children)
                ss << print(child, scoped_comma);
        }
            ss << ">" << comma;
            break;
        case ArgumentType::dictionary_entry:
            ss << comma << "std::pair<";
        {
            ss << print(element->children.front(), Comma())
               << ", "
               << print(element->children.back(), Comma())
               << ">" << comma;
        }
            break;
        default:
            break;
        }

        return ss.str();
    }

    std::string print_type()
    {
        return state.is_void ? "void" : print(state.root, Comma());
    }

    State state;
};

class Interface
{
public:
    static const Interface& empty()
    {
        static const Interface interface{};
        return interface;
    }

    Interface(const std::string& name)
        : d(new Private(name))
    {
    }

    Interface& operator=(const Interface& rhs)
    {
        d = rhs.d;
        return *this;
    }

    bool operator==(const Interface& rhs) const
    {
        return d == rhs.d;
    }

    const std::string& name() const
    {
        return d->name;
    }

private:
    Interface()
    {
    }

    struct Private
    {
        Private(const std::string& name)
            : name(name)
        {
        }

        ~Private()
        {
        }

        std::string name;
    };
    std::shared_ptr<Private> d;
};

class Method
{
public:
    static const Method& empty()
    {
        static const Method method;
        return method;
    }

    Method(const std::string& name)
        : d(new Private(name))
    {
    }

    Method& operator=(const Method& rhs)
    {
        d = rhs.d;
        return *this;
    }

    bool operator==(const Method& rhs) const
    {
        return d == rhs.d;
    }

    const std::string& name() const
    {
        return d->name;
    }

    TypeMangler& in_type_mangler()
    {
        return d->in_type_mangler;
    }

    TypeMangler& out_type_mangler()
    {
        return d->out_type_mangler;
    }

private:
    Method()
    {
    }

    struct Private
    {
        Private(const std::string& name)
            : name(name)
        {
        }

        std::string name;
        TypeMangler in_type_mangler;
        TypeMangler out_type_mangler;
    };
    std::shared_ptr<Private> d;
};

class Property
{
public:
    static const Property& empty()
    {
        static const Property property;
        return property;
    }

    Property(const std::string& name)
        : d(new Private(name))
    {
    }

    Property& operator=(const Property& rhs)
    {
        d = rhs.d;
        return *this;
    }

    bool operator==(const Property& rhs) const
    {
        return d == rhs.d;
    }

    const std::string& name() const
    {
        return d->name;
    }

    TypeMangler& argument_type_mangler()
    {
        return d->argument_type_mangler;
    }

private:
    Property()
    {
    }

    struct Private
    {
        Private(const std::string& name)
            : name(name)
        {
        }

        std::string name;
        TypeMangler argument_type_mangler;
    };
    std::shared_ptr<Private> d;
};

class Signal
{
public:
    static const Signal& empty()
    {
        static const Signal signal;
        return signal;
    }

    Signal(const std::string& name)
        : d(new Private(name))
    {
    }

    Signal& operator=(const Signal& rhs)
    {
        d = rhs.d;
        return *this;
    }

    bool operator==(const Signal& rhs) const
    {
        return d == rhs.d;
    }

    const std::string& name() const
    {
        return d->name;
    }

    TypeMangler& argument_type_mangler()
    {
        return d->argument_type_mangler;
    }

private:
    Signal()
    {
    }

    struct Private
    {
        Private(const std::string& name)
            : name(name)
        {
        }

        std::string name;
        TypeMangler argument_type_mangler;
    };
    std::shared_ptr<Private> d;
};

std::tuple<std::vector<std::string>, std::string> parse_namespaces_and_interface_from_dbus_name(
        const std::string& name)
{
    using boost::algorithm::is_any_of;
    using boost::algorithm::make_split_iterator;

    static const std::string separator = ".";

    auto copy = name;

    boost::algorithm::split_iterator<std::string::iterator> it
            = make_split_iterator(
                copy, boost::algorithm::token_finder(is_any_of(separator.c_str())));
    auto itt(it); itt++;

    std::vector<std::string> namespaces;

    while (!itt.eof())
    {
        namespaces.push_back(std::string(it->begin(), it->end()));
        it++; itt++;
    }

    return std::make_tuple(namespaces, std::string(it->begin(), it->end()));
}
}
struct Generator::Private
{
    struct Context
    {
        struct
        {
            std::size_t count = 0;
            struct
            {
                std::string path;
                struct
                {
                    File protocol_file;
                    std::stringstream buffer;
                    detail::Interface current = detail::Interface::empty();
                    struct
                    {
                        detail::Method current = detail::Method::empty();
                    } method;
                    struct
                    {
                        std::size_t count = 0;
                        detail::Property current = detail::Property::empty();
                    } properties;
                    struct
                    {
                        std::size_t count = 0;
                        detail::Signal current = detail::Signal::empty();
                    } signals;
                } interface;
            } node;
        } namespaces;
        std::stringstream raw_file_buffer;
    };
};

const GeneratorConfiguration& Generator::default_configuration()
{
    static const GeneratorConfiguration config =
    {
        []()
        {
            auto tag = boost::uuids::random_generator()();
            auto tag_string = boost::uuids::to_string(tag);

            boost::algorithm::to_upper(tag_string);
            boost::algorithm::replace_all(tag_string, "-", "_");
            tag_string = "AUTO_" + tag_string;

            return tag_string;
        },
        []()
        {
            auto ts = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            return std::asctime(std::localtime(&ts));
        }
    };

    return config;
}

Generator::Generator() : d(new Private{})
{
}

Generator::~Generator()
{
}

bool Generator::invoke_for_model_with_configuration(
        const std::shared_ptr<Compiler::Element>& element,
        std::istream& raw_file_contents,
        const GeneratorConfiguration& config)
{
    using boost::filesystem::current_path;

    Private::Context context;
    bool state_is_tag_open = false;
    for (auto it = std::istreambuf_iterator<char>(raw_file_contents);
         it != std::istreambuf_iterator<char>();
         ++it)
    {
        auto c = *it;

        switch (c)
        {
        case '<': state_is_tag_open = true; context.raw_file_buffer << c; continue;
        case '>': state_is_tag_open = false; context.raw_file_buffer << c; continue;
        case '"': context.raw_file_buffer << R"(\")"; continue;
        }

        if (std::iscntrl(c))
            continue;

        if (!state_is_tag_open && std::isspace(c))
            continue;

        context.raw_file_buffer << c;
    }

    auto visitor = [&context, &config](const Compiler::Element& element)
    {
        if (element.type() == Compiler::Element::Type::node)
        {
            context.namespaces.node.path = element.node().name;
        } else if (element.type() == Compiler::Element::Type::interface)
        {
            auto name = element.interface().name;

            auto ns_and_interface = detail::parse_namespaces_and_interface_from_dbus_name(name);
            std::vector<std::string> namespaces = std::get<0>(ns_and_interface);
            std::string interface = std::get<1>(ns_and_interface);

            context.namespaces.node.interface.protocol_file.open(
                        current_path() / (interface + ".h"),
                        std::ios_base::out | std::ios_base::trunc);

            context.namespaces.node.interface.buffer
                    << "/*" << std::endl
                    << " * " << "Auto-generated header file." << std::endl
                    << " * " << "Build timestamp: " << config.build_time_stamp_generator() << std::endl
                    << "*/" << std::endl;

            auto include_guard = config.include_guard_generator();

            context.namespaces.node.interface.buffer
                    << "#ifndef " << include_guard << "_H_" << std::endl
                    << "#define " << include_guard << "_H_" << std::endl
                    << std::endl;

            context.namespaces.node.interface.buffer
                    << "#include <org/freedesktop/dbus/types/variant.h>" << std::endl
                    << std::endl
                    << "#include <chrono>" << std::endl
                    << "#include <string>" << std::endl
                    << "#include <tuple>" << std::endl
                    << "#include <vector>" << std::endl
                    << std::endl
                    << "#include <cstdint>" << std::endl
                    << std::endl;

            for (auto ns : namespaces)
            {
                context.namespaces.node.interface.buffer
                        << "namespace " << ns << std::endl
                        << "{" << std::endl;
                context.namespaces.count++;
            }

            context.namespaces.node.interface.buffer
                    << "struct " << interface << std::endl
                    << "{" << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& default_path() { static const std::string s{\""
                    << context.namespaces.node.path
                    << "\"}; return s; }"
                    << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& name() { static const std::string s{\""
                    << element.interface().name
                    << "\"}; return s; }"
                    << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& introspect() { static const std::string s{\""
                    << context.raw_file_buffer.str()
                    << "\"}; return s; }"
                    << std::endl;

            context.namespaces.node.interface.current = detail::Interface{interface};
        } else if (element.type() == Compiler::Element::Type::property)
        {
            // We are stateful here in that we have to check whether
            // any signals are present on this interface. If so, we
            // have to close the enclosing struct, too.
            if (context.namespaces.node.interface.signals.count > 0)
            {
                context.namespaces.node.interface.buffer
                        << "};" << std::endl;
            }

            // We are stateful here in that we have to check whether
            // any properties have been visited before. If so, we
            // have to open the enclosing struct, too.
            if (context.namespaces.node.interface.properties.count == 0)
            {
                context.namespaces.node.interface.buffer
                        << "struct Properties" << std::endl
                        << "{" << std::endl;
            }

            context.namespaces.node.interface.properties.count++;
            context.namespaces.node.interface.properties.current
                    = detail::Property(element.property().name);

            context.namespaces.node.interface.buffer
                    << "struct " << element.property().name << std::endl
                    << "{" << std::endl;
            context.namespaces.node.interface.buffer
                    << "typedef " << context.namespaces.node.interface.current.name() << " Interface;" << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& name() { static const std::string s{\""
                    << element.property().name
                    << "\"}; return s; }"
                    << std::endl;

            switch(element.property().access)
            {
            case IntrospectionParser::Property::Access::read:
                context.namespaces.node.interface.buffer
                        << "static const bool readable = true;" << std::endl
                        << "static const bool writable = false;" << std::endl;
                break;
            case IntrospectionParser::Property::Access::write:
                context.namespaces.node.interface.buffer
                        << "static const bool readable = false;" << std::endl
                        << "static const bool writable = true;" << std::endl;
                break;
            case IntrospectionParser::Property::Access::read_write:
                context.namespaces.node.interface.buffer
                        << "static const bool readable = true;" << std::endl
                        << "static const bool writable = true;" << std::endl;
                break;
            }

            context.
                    namespaces.
                    node.
                    interface.
                    properties.
                    current.
                    argument_type_mangler().update_from_signature(
                        element.property().type);
            context.namespaces.node.interface.buffer
                    << "typedef "
                    << context.namespaces.node.interface.properties.current.argument_type_mangler().print_type()
                    << " ArgumentType;"
                    << std::endl;
        } else if (element.type() == Compiler::Element::Type::method)
        {
            context.namespaces.node.interface.buffer
                    << "struct " << element.method().name << std::endl
                    << "{" << std::endl;
            context.namespaces.node.interface.buffer
                    << "typedef " << context.namespaces.node.interface.current.name() << " Interface;" << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& name() { static const std::string s{\""
                    << element.method().name
                    << "\"}; return s; }"
                    << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::chrono::milliseconds& default_timeout() { static const std::chrono::seconds s{10}; return s; }"
                    << std::endl;

            context.namespaces.node.interface.method.current = detail::Method(element.method().name);
        } else if (element.type() == Compiler::Element::Type::signal)
        {
            // We are stateful here in that we have to check whether
            // any signals have been visited before. If so, we
            // have to open the enclosing struct, too.
            if (context.namespaces.node.interface.signals.count == 0)
            {
                context.namespaces.node.interface.buffer
                        << "struct Signals" << std::endl
                        << "{" << std::endl;
            }

            context.namespaces.node.interface.signals.count++;
            context.namespaces.node.interface.signals.current
                    = detail::Signal(element.signal().name);

            context.namespaces.node.interface.buffer
                    << "struct " << element.signal().name << std::endl
                    << "{" << std::endl;
            context.namespaces.node.interface.buffer
                    << "typedef " << context.namespaces.node.interface.current.name() << " Interface;" << std::endl;
            context.namespaces.node.interface.buffer
                    << "static const std::string& name() { static const std::string s{\"" << element.signal().name << "\"}; return s; }" << std::endl;
        } else if (element.type() == Compiler::Element::Type::argument)
        {
            if (!(context.namespaces.node.interface.method.current == detail::Method::empty()))
            {
                switch(element.argument().direction)
                {
                case IntrospectionParser::Argument::Direction::in:
                    context.namespaces.node.interface.method.current.in_type_mangler().update_from_signature(
                                element.argument().type.begin(),
                                element.argument().type.end());
                    break;
                case IntrospectionParser::Argument::Direction::out:
                    context.namespaces.node.interface.method.current.out_type_mangler().update_from_signature(
                                element.argument().type.begin(),
                                element.argument().type.end());
                    break;
                case IntrospectionParser::Argument::Direction::context:
                    context.namespaces.node.interface.method.current.in_type_mangler().update_from_signature(
                                element.argument().type.begin(),
                                element.argument().type.end());
                    context.namespaces.node.interface.method.current.out_type_mangler().update_from_signature(
                                element.argument().type.begin(),
                                element.argument().type.end());
                    break;
                }
            } else if (!(context.namespaces.node.interface.signals.current == detail::Signal::empty()))
            {
                context.namespaces.node.interface.signals.current.argument_type_mangler().update_from_signature(element.argument().type);
            }
        }
    };

    auto post_visitor = [&context](const Compiler::Element& element)
    {
        if (element.type() == Compiler::Element::Type::interface)
        {
            // We are stateful here in that we have to check whether
            // any properties are present on this interface. If so, we
            // have to close the enclosing struct, too.
            if (context.namespaces.node.interface.properties.count > 0)
                context.namespaces.node.interface.buffer
                        << "};" << std::endl;

            context.namespaces.node.interface.buffer
                    << "};" << std::endl;
            for (unsigned int i = 0; i < context.namespaces.count; i++)
            {
                context.namespaces.node.interface.buffer
                        << "}" << std::endl;
            }
            context.namespaces.node.interface.buffer
                    << "#endif" << std::endl;
        } else if (element.type() == Compiler::Element::Type::property)
        {
            context.namespaces.node.interface.buffer
                    << "};" << std::endl;
            context.namespaces.node.interface.properties.current = detail::Property::empty();
        } else if (element.type() == Compiler::Element::Type::method)
        {
            context.namespaces.node.interface.buffer
                    << "typedef " << context.namespaces.node.interface.method.current.in_type_mangler().print_type() << " ArgumentType;" << std::endl
                    << "typedef " << context.namespaces.node.interface.method.current.out_type_mangler().print_type() << " ResultType;" << std::endl
                    << "};" << std::endl;
            context.namespaces.node.interface.method.current = detail::Method::empty();
        } else if (element.type() == Compiler::Element::Type::signal)
        {
            context.namespaces.node.interface.buffer
                    << "typedef " << context.namespaces.node.interface.signals.current.argument_type_mangler().print_type() << " ArgumentType;" << std::endl
                    << "};" << std::endl;
            context.namespaces.node.interface.signals.current
                    = detail::Signal::empty();
        }
    };

    element->apply(
                std::function<void(const Compiler::Element&)>(),
                visitor,
                post_visitor);

    context.namespaces.node.interface.protocol_file << context.namespaces.node.interface.buffer.str();

    return true;
}
}
}
}
