#include "test.h"

#include <pluribus/core/core.h>

#include <limits>
#include <string>

namespace {

using namespace pluribus;

RequestId make_request_id() {
    return *RequestId::parse("11111111-2222-4333-8444-555555555555");
}

ExecutionSpec make_spec() {
    ExecutionSpec spec;
    spec.request_id = make_request_id();
    spec.executable = *ExecutableRef::from_package("hello");
    return spec;
}

std::string artifact_text(char digit) {
    return std::string("sha256:") + std::string(64, digit);
}

} // namespace

PLURIBUS_TEST(ids_uuid_round_trip) {
    const auto parsed = Uuid::parse("01234567-89ab-cdef-0123-456789abcdef");
    CHECK(parsed.has_value());
    CHECK(parsed->to_string() == "01234567-89ab-cdef-0123-456789abcdef");
    CHECK(!parsed->is_nil());
}

PLURIBUS_TEST(ids_uuid_accepts_upper_case_and_canonicalizes) {
    const auto parsed = Uuid::parse("01234567-89AB-CDEF-0123-456789ABCDEF");
    CHECK(parsed.has_value());
    CHECK(parsed->to_string() == "01234567-89ab-cdef-0123-456789abcdef");
}

PLURIBUS_TEST(ids_uuid_rejects_malformed) {
    CHECK(!Uuid::parse("").has_value());
    CHECK(!Uuid::parse("not-a-uuid").has_value());
    CHECK(!Uuid::parse("{01234567-89ab-cdef-0123-456789abcdef}").has_value());
    CHECK(!Uuid::parse("01234567-89ab-cdef-0123-456789abcde").has_value());
    CHECK(!Uuid::parse("01234567x89ab-cdef-0123-456789abcdef").has_value());
}

PLURIBUS_TEST(ids_uuid_generate_is_version_4_and_not_nil) {
    const auto uuid = Uuid::generate();
    CHECK(!uuid.is_nil());
    const auto text = uuid.to_string();
    CHECK(text.size() == 36);
    CHECK(text[14] == '4');
    CHECK(text[19] == '8' || text[19] == '9' || text[19] == 'a' || text[19] == 'b');
    CHECK(Uuid::parse(text).has_value());
}

PLURIBUS_TEST(ids_artifact_round_trip) {
    const auto text = artifact_text('a');
    const auto artifact = ArtifactId::parse(text);
    CHECK(artifact.has_value());
    CHECK(artifact->to_string() == text);
    CHECK(!artifact->is_nil());
}

PLURIBUS_TEST(ids_artifact_rejects_non_canonical) {
    CHECK(!ArtifactId::parse("").has_value());
    CHECK(!ArtifactId::parse("sha256:").has_value());
    CHECK(!ArtifactId::parse(artifact_text('A')).has_value());
    CHECK(!ArtifactId::parse("sha512:" + std::string(64, 'a')).has_value());
    CHECK(!ArtifactId::parse(std::string("sha256:") + std::string(63, 'a')).has_value());
    CHECK(!ArtifactId::parse(std::string("sha256:") + std::string(65, 'a')).has_value());
}

PLURIBUS_TEST(ids_sha256_digest_round_trip) {
    const auto digest = Sha256Digest::parse_hex(std::string(64, 'f'));
    CHECK(digest.has_value());
    CHECK(digest->to_hex() == std::string(64, 'f'));
    CHECK(!Sha256Digest::parse_hex(std::string(63, 'f')).has_value());
}

PLURIBUS_TEST(names_capability_validation) {
    CHECK(CapabilityName::parse("gpu.cuda").has_value());
    CHECK(CapabilityName::parse("target.thor").has_value());
    CHECK(CapabilityName::parse("sensor.camera.360").has_value());
    CHECK(CapabilityName::parse("x86_64").has_value());

    CHECK(!CapabilityName::parse("").has_value());
    CHECK(!CapabilityName::parse("gpu..cuda").has_value());
    CHECK(!CapabilityName::parse(".gpu").has_value());
    CHECK(!CapabilityName::parse("gpu.").has_value());
    CHECK(!CapabilityName::parse("gpu/cuda").has_value());
    CHECK(!CapabilityName::parse("gpu cuda").has_value());
}

PLURIBUS_TEST(names_service_validation) {
    CHECK(ServiceName::parse("radarscape/sar/output").has_value());
    CHECK(ServiceName::parse("radar-slave/control").has_value());
    CHECK(ServiceName::parse("output").has_value());

    CHECK(!ServiceName::parse("").has_value());
    CHECK(!ServiceName::parse("/output").has_value());
    CHECK(!ServiceName::parse("output/").has_value());
    CHECK(!ServiceName::parse("radar//output").has_value());
    CHECK(!ServiceName::parse("radar/../output").has_value());
    CHECK(!ServiceName::parse("host:5800/output").has_value());
    CHECK(!ServiceName::parse("radar output").has_value());
}

PLURIBUS_TEST(executable_parses_supported_sources) {
    const auto package = ExecutableRef::parse("hello");
    CHECK(package.has_value());
    CHECK(package->is_package());
    CHECK(package->package_name() == "hello");
    CHECK(!package->package_version().has_value());
    CHECK(package->to_string() == "hello");

    const auto versioned = ExecutableRef::parse("hello@1.4.2");
    CHECK(versioned.has_value());
    CHECK(versioned->is_package());
    CHECK(versioned->package_version().has_value());
    CHECK(*versioned->package_version() == "1.4.2");
    CHECK(versioned->to_string() == "hello@1.4.2");

    const auto artifact = ExecutableRef::parse("artifact:" + artifact_text('b'));
    CHECK(artifact.has_value());
    CHECK(artifact->is_artifact());
    CHECK(artifact->artifact().to_string() == artifact_text('b'));

    const auto client_file = ExecutableRef::parse("./hello");
    CHECK(client_file.has_value());
    CHECK(client_file->is_client_file());
    CHECK(client_file->client_path() == "./hello");
    CHECK(client_file->to_string() == "./hello");
}

PLURIBUS_TEST(executable_rejects_invalid_sources) {
    CHECK(!ExecutableRef::parse("").has_value());
    CHECK(!ExecutableRef::parse("hello@").has_value());
    CHECK(!ExecutableRef::parse("hello@@1.0").has_value());
    CHECK(!ExecutableRef::parse("hello world").has_value());
    CHECK(!ExecutableRef::parse("hello/other").has_value());
    CHECK(!ExecutableRef::parse("../hello").has_value());
    CHECK(!ExecutableRef::parse("./../hello").has_value());
    CHECK(!ExecutableRef::parse("./").has_value());
    CHECK(!ExecutableRef::parse("artifact:sha256:" + std::string(64, 'A')).has_value());
}

PLURIBUS_TEST(execution_spec_accepts_valid_model) {
    auto spec = make_spec();
    spec.arguments = {"--flag", "value"};
    spec.environment = {{"RADAR_MODE", "test"}};
    spec.working_directory = "work";
    spec.inputs.push_back(InputSpec{*ArtifactId::parse(artifact_text('a')), "input/data.bin"});
    spec.outputs.push_back(OutputSpec{"output/result.bin"});
    spec.services.push_back(*ServiceName::parse("radarscape/sar/output"));
    spec.labels = {{"pipeline", "sar"}};
    CHECK(spec.validate().has_value());
}

PLURIBUS_TEST(execution_spec_rejects_invalid_model) {
    {
        auto spec = make_spec();
        spec.request_id = RequestId{};
        CHECK(!spec.validate().has_value());
        CHECK(spec.validate().error().code == ErrorCode::invalid_argument);
    }
    {
        auto spec = make_spec();
        spec.executable = ExecutableRef{};
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.working_directory = "/absolute";
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.inputs.push_back(InputSpec{*ArtifactId::parse(artifact_text('a')), "../secret"});
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        const auto artifact = *ArtifactId::parse(artifact_text('a'));
        spec.inputs.push_back(InputSpec{artifact, "input/file"});
        spec.inputs.push_back(InputSpec{artifact, "input/file"});
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.outputs.push_back(OutputSpec{"output/../escape"});
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.environment = {{"BAD-NAME", "value"}};
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.resources.cpu = -1;
        CHECK(!spec.validate().has_value());
    }
    {
        auto spec = make_spec();
        spec.arguments = {std::string("value\0hidden", 12)};
        CHECK(!spec.validate().has_value());
    }
}

PLURIBUS_TEST(paths_reject_traversal_and_host_forms) {
    CHECK(is_sandbox_relative_path("input/file"));
    CHECK(is_sandbox_relative_path("a/b/c"));
    CHECK(is_sandbox_relative_path("file.txt"));

    CHECK(!is_sandbox_relative_path(""));
    CHECK(!is_sandbox_relative_path("/absolute"));
    CHECK(!is_sandbox_relative_path("C:/windows"));
    CHECK(!is_sandbox_relative_path("a\\b"));
    CHECK(!is_sandbox_relative_path("a/../b"));
    CHECK(!is_sandbox_relative_path("./a"));
    CHECK(!is_sandbox_relative_path("a//b"));
    CHECK(!is_sandbox_relative_path("a/b/"));
    CHECK(!is_sandbox_relative_path("a/./b"));
}

PLURIBUS_TEST(execution_states_round_trip) {
    for (const auto state : {
             ExecutionState::created,
             ExecutionState::queued,
             ExecutionState::assigned,
             ExecutionState::materializing,
             ExecutionState::starting,
             ExecutionState::running,
             ExecutionState::completed,
             ExecutionState::failed,
             ExecutionState::terminated,
             ExecutionState::lost,
         }) {
        CHECK(parse_execution_state(to_string(state)) == state);
    }
    CHECK(!parse_execution_state("Unknown").has_value());
    CHECK(to_string(ExecutionState::running) == "Running");
}

PLURIBUS_TEST(execution_states_classify_terminality) {
    CHECK(!is_definitive_terminal(ExecutionState::running));
    CHECK(!is_terminal_observation(ExecutionState::running));
    CHECK(is_definitive_terminal(ExecutionState::completed));
    CHECK(is_definitive_terminal(ExecutionState::failed));
    CHECK(is_definitive_terminal(ExecutionState::terminated));
    CHECK(!is_definitive_terminal(ExecutionState::lost));
    CHECK(is_terminal_observation(ExecutionState::lost));
}

PLURIBUS_TEST(execution_states_allow_documented_transitions) {
    CHECK(can_transition(ExecutionState::created, ExecutionState::queued));
    CHECK(can_transition(ExecutionState::created, ExecutionState::failed));
    CHECK(can_transition(ExecutionState::created, ExecutionState::terminated));
    CHECK(can_transition(ExecutionState::queued, ExecutionState::assigned));
    CHECK(can_transition(ExecutionState::assigned, ExecutionState::materializing));
    CHECK(can_transition(ExecutionState::materializing, ExecutionState::starting));
    CHECK(can_transition(ExecutionState::starting, ExecutionState::running));
    CHECK(can_transition(ExecutionState::running, ExecutionState::completed));
    CHECK(can_transition(ExecutionState::running, ExecutionState::failed));
    CHECK(can_transition(ExecutionState::running, ExecutionState::terminated));
    CHECK(can_transition(ExecutionState::running, ExecutionState::lost));
}

PLURIBUS_TEST(execution_states_allow_lost_reconciliation) {
    CHECK(can_transition(ExecutionState::lost, ExecutionState::assigned));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::materializing));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::starting));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::running));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::completed));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::failed));
    CHECK(can_transition(ExecutionState::lost, ExecutionState::terminated));
}

PLURIBUS_TEST(execution_states_reject_invalid_transitions) {
    CHECK(!can_transition(ExecutionState::created, ExecutionState::running));
    CHECK(!can_transition(ExecutionState::queued, ExecutionState::running));
    CHECK(!can_transition(ExecutionState::running, ExecutionState::queued));
    CHECK(!can_transition(ExecutionState::completed, ExecutionState::running));
    CHECK(!can_transition(ExecutionState::failed, ExecutionState::lost));
    CHECK(!can_transition(ExecutionState::terminated, ExecutionState::completed));
    CHECK(!can_transition(ExecutionState::lost, ExecutionState::queued));

    const auto result = validate_transition(ExecutionState::completed, ExecutionState::running);
    CHECK(!result.has_value());
    CHECK(result.error().code == ErrorCode::conflict);
}

PLURIBUS_TEST(node_health_round_trip) {
    for (const auto health : {NodeHealth::healthy, NodeHealth::degraded, NodeHealth::unhealthy}) {
        CHECK(parse_node_health(to_string(health)) == health);
    }
    CHECK(!parse_node_health("unknown").has_value());
}

PLURIBUS_TEST(resources_abi_and_architecture) {
    const auto abi = AbiVersion::parse("pluribus-v1");
    CHECK(abi.has_value());
    CHECK(abi->to_string() == "pluribus-v1");
    CHECK(!AbiVersion::parse("").has_value());
    CHECK(!AbiVersion::parse("pluribus v1").has_value());

    CHECK(parse_cpu_architecture("x86_64") == CpuArchitecture::x86_64);
    CHECK(parse_cpu_architecture("arm64") == CpuArchitecture::arm64);
    CHECK(!parse_cpu_architecture("x86").has_value());

    const CompatibilityKey key{*abi, CpuArchitecture::arm64};
    CHECK(key.to_string() == "pluribus-v1-arm64");
}

PLURIBUS_TEST(errors_have_wire_names) {
    CHECK(to_wire_code(ErrorCode::invalid_argument) == "INVALID_ARGUMENT");
    CHECK(to_wire_code(ErrorCode::launch_failed) == "LAUNCH_FAILED");
    CHECK(to_wire_code(ErrorCode::internal) == "INTERNAL");
}
