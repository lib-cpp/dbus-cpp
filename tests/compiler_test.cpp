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

#include <core/dbus/compiler.h>
#include <core/dbus/generator.h>
#include <core/dbus/generator_configuration.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>

#include <fstream>

// local includes to access testing data
#include "test_data.h"

namespace dbus = core::dbus;

namespace
{
struct ParserInspector
{
    MOCK_METHOD1(on_node, void(const dbus::IntrospectionParser::Node&));
    MOCK_METHOD0(on_node_done, void());
    MOCK_METHOD1(on_interface, void(const dbus::IntrospectionParser::Interface&));
    MOCK_METHOD0(on_interface_done, void());
    MOCK_METHOD1(on_method, void(const dbus::IntrospectionParser::Method&));
    MOCK_METHOD0(on_method_done, void());
    MOCK_METHOD1(on_property, void(const dbus::IntrospectionParser::Property&));
    MOCK_METHOD1(on_signal, void(const dbus::IntrospectionParser::Signal&));
    MOCK_METHOD0(on_signal_done, void());
    MOCK_METHOD1(on_argument, void(const dbus::IntrospectionParser::Argument&));
    MOCK_METHOD0(on_argument_done, void());
    MOCK_METHOD1(on_annotation, void(const dbus::IntrospectionParser::Annotation&));
    MOCK_METHOD0(on_annotation_done, void());
};

struct MockGenerator : public dbus::Generator
{
    MOCK_METHOD3(invoke_for_model_with_configuration,
                 bool(
                     const std::shared_ptr<dbus::Compiler::Element>&,
                     std::istream&,
                     const dbus::GeneratorConfiguration&));
};

void ensure_test_introspection_file(const std::string& fn)
{
    std::ofstream out(fn.c_str());
    out << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"" << std::endl
        << "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">" << std::endl
        << "<node name=\"/core/sample_object\">" << std::endl
        << "  <interface name=\"org.freedesktop.SampleInterface\">" << std::endl
        << "    <method name=\"Frobate\">" << std::endl
        << "      <arg name=\"foo\" type=\"i\" direction=\"in\"/>" << std::endl
        << "      <arg name=\"bar\" type=\"s\" direction=\"out\"/>" << std::endl
        << "      <arg name=\"baz\" type=\"a{us}\" direction=\"out\"/>" << std::endl
        << "      <annotation name=\"org.freedesktop.DBus.Deprecated\" value=\"true\"/>" << std::endl
        << "    </method>" << std::endl
        << "    <method name=\"Bazify\">" << std::endl
        << "      <arg name=\"bar\" type=\"(iiu)\" direction=\"in\"/>" << std::endl
        << "      <arg name=\"bar\" type=\"v\" direction=\"out\"/>" << std::endl
        << "    </method>" << std::endl
        << "    <method name=\"Mogrify\">" << std::endl
        << "      <arg name=\"bar\" type=\"(iiav)\" direction=\"in\"/>" << std::endl
        << "    </method>" << std::endl
        << "    <signal name=\"Changed\">" << std::endl
        << "      <arg name=\"new_value\" type=\"b\"/>" << std::endl
        << "    </signal>" << std::endl
        << "    <property name=\"Bar\" type=\"y\" access=\"readwrite\"/>" << std::endl
        << "  </interface>" << std::endl
        << "  <node name=\"child_of_sample_object\"/>" << std::endl
        << "  <node name=\"another_child_of_sample_object\"/>" << std::endl
        << " </node>" << std::endl;
}
}

TEST(IntrospectionParser, invoking_parser_reports_elements_correctly)
{
    using namespace ::testing;
    
    const std::string tmp_fn(__PRETTY_FUNCTION__);
    std::remove(tmp_fn.c_str());

    ensure_test_introspection_file(tmp_fn);
    
    dbus::IntrospectionParser parser;
    NiceMock<ParserInspector> inspector;

    parser.on_node(std::bind(&ParserInspector::on_node, &inspector, std::placeholders::_1));
    parser.on_node_done(std::bind(&ParserInspector::on_node_done, &inspector));
    parser.on_interface(std::bind(&ParserInspector::on_interface, &inspector, std::placeholders::_1));
    parser.on_interface_done(std::bind(&ParserInspector::on_interface_done, &inspector));
    parser.on_method(std::bind(&ParserInspector::on_method, &inspector, std::placeholders::_1));
    parser.on_method_done(std::bind(&ParserInspector::on_method_done, &inspector));
    parser.on_property(std::bind(&ParserInspector::on_property, &inspector, std::placeholders::_1));
    parser.on_signal(std::bind(&ParserInspector::on_signal, &inspector, std::placeholders::_1));
    parser.on_signal_done(std::bind(&ParserInspector::on_signal_done, &inspector));
    parser.on_argument(std::bind(&ParserInspector::on_argument, &inspector, std::placeholders::_1));
    parser.on_argument_done(std::bind(&ParserInspector::on_argument_done, &inspector));
    parser.on_annotation(std::bind(&ParserInspector::on_annotation, &inspector, std::placeholders::_1));
    parser.on_annotation_done(std::bind(&ParserInspector::on_annotation_done, &inspector));

    EXPECT_CALL(inspector, on_node(_)).Times(Exactly(3));
    EXPECT_CALL(inspector, on_node_done()).Times(Exactly(3));
    EXPECT_CALL(inspector, on_interface(_)).Times(Exactly(1));
    EXPECT_CALL(inspector, on_interface_done()).Times(Exactly(1));
    EXPECT_CALL(inspector, on_method(_)).Times(Exactly(3));
    EXPECT_CALL(inspector, on_method_done()).Times(Exactly(3));
    EXPECT_CALL(inspector, on_property(_)).Times(Exactly(1));
    EXPECT_CALL(inspector, on_signal(_)).Times(Exactly(1));
    EXPECT_CALL(inspector, on_signal_done()).Times(Exactly(1));
    EXPECT_CALL(inspector, on_argument(_)).Times(Exactly(7));
    EXPECT_CALL(inspector, on_annotation(_)).Times(Exactly(1));

    EXPECT_TRUE(parser.invoke_for(tmp_fn));
}

TEST(IntrospectionCompiler, invoking_the_compiler_triggers_the_generator)
{
    using namespace ::testing;

    const std::string tmp_fn(__PRETTY_FUNCTION__);
    std::remove(tmp_fn.c_str());

    ensure_test_introspection_file(tmp_fn);

    auto parser = std::make_shared<dbus::IntrospectionParser>();

    NiceMock<MockGenerator> generator;
    EXPECT_CALL(generator, invoke_for_model_with_configuration(_, _, _));
    
    dbus::Compiler compiler(parser,
                            std::shared_ptr<dbus::Generator>(&generator,
                                                             [](dbus::Generator*){}));
    
    EXPECT_NO_THROW(compiler.process_introspection_file_with_generator_config(
                        tmp_fn,
                        dbus::Generator::default_configuration()));
}

namespace
{
struct StubGenerator : public dbus::Generator
{
    int node_count = 0;
    int interface_count = 0;
    int method_count = 0;
    int signal_count = 0;
    int property_count = 0;
    int annotation_count = 0;
    int argument_count = 0;

    bool invoke_for_model_with_configuration(
            const std::shared_ptr<dbus::Compiler::Element>& element,
            std::istream&,
            const dbus::GeneratorConfiguration&)
    {
        auto visitor = [this](const dbus::Compiler::Element& element)
        {
            switch(element.type())
            {
                case dbus::Compiler::Element::Type::node:
                node_count++;
                break;
                case dbus::Compiler::Element::Type::interface:
                interface_count++;
                break;
                case dbus::Compiler::Element::Type::method:
                method_count++;
                break;
                case dbus::Compiler::Element::Type::signal:
                signal_count++;
                break;
                case dbus::Compiler::Element::Type::property:
                property_count++;
                break;
                case dbus::Compiler::Element::Type::argument:
                argument_count++;
                break;
                case dbus::Compiler::Element::Type::annotation:
                annotation_count++;
                break;
            }
        };

        static const std::function<void(const dbus::Compiler::Element&)> ignored{};

        element->apply(ignored, visitor, ignored);

        return true;
    }
};
}

TEST(IntrospectionGenerator, receives_correct_tree_from_compiler)
{
    using namespace ::testing;

    const std::string tmp_fn(__PRETTY_FUNCTION__);
    std::remove(tmp_fn.c_str());

    ensure_test_introspection_file(tmp_fn);

    auto parser = std::make_shared<dbus::IntrospectionParser>();
    auto generator = std::make_shared<StubGenerator>();

    dbus::Compiler compiler(parser, generator);
    
    EXPECT_NO_THROW(compiler.process_introspection_file_with_generator_config(
                        tmp_fn,
                        dbus::Generator::default_configuration()));

    EXPECT_EQ(3, generator->node_count);
    EXPECT_EQ(1, generator->interface_count);
    EXPECT_EQ(3, generator->method_count);
    EXPECT_EQ(1, generator->signal_count);
    EXPECT_EQ(1, generator->property_count);
    EXPECT_EQ(7, generator->argument_count);
    EXPECT_EQ(1, generator->annotation_count);
}

TEST(IntrospectionGenerator, generates_correct_protocol_definition_header_file)
{
    using namespace ::testing;

    const std::string tmp_fn(__PRETTY_FUNCTION__);
    std::remove(tmp_fn.c_str());

    ensure_test_introspection_file(tmp_fn);

    auto parser = std::make_shared<dbus::IntrospectionParser>();
    auto generator = std::make_shared<dbus::Generator>();

    dbus::Compiler compiler(parser, generator);

    EXPECT_NO_THROW(compiler.process_introspection_file_with_generator_config(
                        tmp_fn,
                        dbus::Generator::default_configuration()));

    const std::string protocol_header_file_name{"SampleInterface.h"};
}

TEST(CompilerMain, generates_correct_protocol_definition_header_file_for_com_canonical_user_metrics)
{
    const char* argv[] =
    {
        "dbus-cppc",
        testing::com::canonical::user_metrics_introspection_file()
    };

    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::com::canonical::url_dispatcher_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::cdma_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::firmware_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::card_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::contact_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::hso_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::network_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::sms_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));

    argv[1] = testing::org::freedesktop::modem_manager::modem::gsm::ussd_introspection_file();
    EXPECT_EQ(EXIT_SUCCESS, dbus::Compiler::main(2, argv));
}
