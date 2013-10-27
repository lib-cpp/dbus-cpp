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

#include <org/freedesktop/dbus/compiler.h>
#include <org/freedesktop/dbus/generator.h>

#include <boost/program_options.hpp>

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

bool Compiler::process_introspection_file_with_generator_config(
        const std::string& fn,
        const GeneratorConfiguration& config = Generator::default_configuration())
{
    d->context.reset();

    if (!d->parser->invoke_for(fn))
        return false;

    std::ifstream in_file(fn);
    if (!in_file)
        return false;

    if (!d->generator->invoke_for_model_with_configuration(d->context.root, in_file, config))
        return false;

    return true;
}

namespace
{
struct CommandLineOptions
{
    static const char* key_input_file() { return "input-file"; }

    CommandLineOptions() : allowed_options("Allowed options")
    {
        static const int unlimited = -1;

        allowed_options.add_options()
                (CommandLineOptions::key_input_file(),
                 boost::program_options::value<std::vector<std::string>>(),
                 "DBUS XML introspection input files to the compiler");
        positional_options.add(CommandLineOptions::key_input_file(), unlimited);
    }

    bool parse(int argc, const char* argv[])
    {
        try
        {
            boost::program_options::variables_map vm;

            boost::program_options::store(
                        boost::program_options::command_line_parser(
                            argc,
                            argv).options(allowed_options).positional(positional_options).run(), vm);
            boost::program_options::notify(vm);

            files = vm[CommandLineOptions::key_input_file()].as<std::vector<std::string>>();
        } catch(...)
        {
            return false;
        }

        return true;
    }

    std::string usage() const
    {
        std::stringstream ss; ss << allowed_options;
        return ss.str();
    }

    boost::program_options::options_description allowed_options;
    boost::program_options::positional_options_description positional_options;

    std::vector<std::string> files;
};

struct AnsiiControlSequences
{
    struct Style
    {
        static constexpr const char* Bold()
        {
            return "\033[1m";
        }

        static constexpr const char* Normal()
        {
            return "\033[0m";
        }
    };

    struct Colors
    {
        static constexpr const char* DefaultForeground()
        {
            return "\033[39m";
        }

        static constexpr const char* Green()
        {
            return "\033[32m";
        }

        static constexpr const char* Red()
        {
            return "\033[91m";
        }
    };
};

struct Ok
{
    friend std::ostream& operator<<(std::ostream& out, const Ok&)
    {
        out << AnsiiControlSequences::Style::Bold()
            << AnsiiControlSequences::Colors::Green()
            << "[OK  ]"
            << AnsiiControlSequences::Colors::DefaultForeground()
            << AnsiiControlSequences::Style::Normal();

        return out;
    }
};

struct Fail
{
    friend std::ostream& operator<<(std::ostream& out, const Fail&)
    {
        out << AnsiiControlSequences::Style::Bold()
            << AnsiiControlSequences::Colors::Red()
            << "[FAIL]"
            << AnsiiControlSequences::Colors::DefaultForeground()
            << AnsiiControlSequences::Style::Normal();

        return out;
    }
};
}

int Compiler::main(int argc, const char* argv[])
{
    CommandLineOptions cli_options;

    if (!cli_options.parse(argc, argv))
    {
        std::cout << "Could not parse command line arguments, aborting now." << std::endl;
        std::cout << cli_options.usage() << std::endl;

        return EXIT_FAILURE;
    }
    auto parser = std::make_shared<dbus::IntrospectionParser>();
    auto generator = std::make_shared<dbus::Generator>();

    dbus::Compiler compiler(parser, generator);

    for (auto file : cli_options.files)
    {
        auto result
                = compiler.process_introspection_file_with_generator_config(
                    file,
                    dbus::Generator::default_configuration());

        if (result)
            std::cout << Ok() << " ";
        else
            std::cout << Fail() << " ";
        std::cout << file << std::endl;

        if (!result)
            exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
}
}
}

