#include "control.capnp.h"
#include "control.pb.h"
#include "node_descriptor_generated.h"
#include "submit_generated.h"

#include <capnp/message.h>
#include <capnp/serialize.h>
#include <flatbuffers/flatbuffers.h>
#include <kj/array.h>
#include <kj/string.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pluribus::bench {
namespace {

using Clock = std::chrono::steady_clock;

volatile std::size_t g_size_sink = 0;
volatile std::uint64_t g_checksum_sink = 0;

struct KeyValueData {
    std::string key;
    std::string value;
};

struct ArtifactInputData {
    std::string artifact;
    std::string path;
    bool read_write = false;
};

struct SubmitData {
    std::string request_id;
    std::string executable;
    std::vector<std::string> arguments;
    std::vector<KeyValueData> environment;
    std::vector<KeyValueData> labels;
    std::vector<ArtifactInputData> inputs;
    std::vector<std::string> outputs;
    int lifetime = 0;
    std::vector<std::string> services;
    std::int64_t cpu_millicores = 0;
    std::uint64_t memory_bytes = 0;
    std::string architecture;
    std::string abi;
    std::vector<std::string> required_capabilities;
    std::vector<std::string> required_devices;
    std::string explicit_node;
};

struct NodeDescriptorData {
    std::string id;
    std::string name;
    std::string abi;
    std::string architecture;
    std::uint32_t logical_cpu_count = 0;
    std::uint64_t total_memory = 0;
    std::vector<std::string> capabilities;
    std::vector<std::string> devices;
    std::uint32_t cpu_load_basis_points = 0;
    std::uint64_t available_memory = 0;
    std::uint32_t running_executions = 0;
};

std::string padded(std::string prefix, std::size_t index, bool large) {
    std::string result = std::move(prefix) + std::to_string(index);
    if (large) {
        result.append(48, 'x');
    }
    return result;
}

SubmitData make_submit(bool large) {
    const std::size_t argument_count = large ? 64 : 8;
    const std::size_t environment_count = large ? 64 : 8;
    const std::size_t label_count = large ? 16 : 4;
    const std::size_t input_count = large ? 64 : 4;
    const std::size_t output_count = large ? 16 : 2;
    const std::size_t service_count = large ? 8 : 1;
    const std::size_t capability_count = large ? 32 : 4;
    const std::size_t device_count = large ? 16 : 1;

    SubmitData data;
    data.request_id = large
        ? "22222222-3333-4444-8555-666666666666-extra-request-identifier"
        : "22222222-3333-4444-8555-666666666666";
    data.executable = large ? "radarscape-sar-worker@1.4.2" : "hello";
    data.architecture = "x86_64";
    data.abi = "pluribus-v1";
    data.lifetime = large ? 1 : 0;
    data.cpu_millicores = 1000;
    data.memory_bytes = 1ULL << 30;
    data.explicit_node = "00000000-0000-0000-0000-000000000001";

    for (std::size_t index = 0; index < argument_count; ++index) {
        data.arguments.push_back(padded("--argument-", index, large));
    }
    for (std::size_t index = 0; index < environment_count; ++index) {
        data.environment.push_back(
            KeyValueData{padded("PLURIBUS_ENV_", index, large),
                         padded("value-", index, large)});
    }
    for (std::size_t index = 0; index < label_count; ++index) {
        data.labels.push_back(
            KeyValueData{padded("label-", index, large),
                         padded("label-value-", index, large)});
    }
    for (std::size_t index = 0; index < input_count; ++index) {
        data.inputs.push_back(ArtifactInputData{
            "sha256:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
            padded("input/file-", index, large),
            index % 2 == 0});
    }
    for (std::size_t index = 0; index < output_count; ++index) {
        data.outputs.push_back(padded("output/result-", index, large));
    }
    for (std::size_t index = 0; index < service_count; ++index) {
        data.services.push_back(padded("radarscape/service-", index, large));
    }
    for (std::size_t index = 0; index < capability_count; ++index) {
        data.required_capabilities.push_back(padded("gpu.capability-", index, large));
    }
    for (std::size_t index = 0; index < device_count; ++index) {
        data.required_devices.push_back(padded("device.sensor-", index, large));
    }
    return data;
}

NodeDescriptorData make_node(bool large) {
    NodeDescriptorData data;
    data.id = "00000000-0000-0000-0000-000000000001";
    data.name = "worker-1";
    data.abi = "pluribus-v1";
    data.architecture = "x86_64";
    data.logical_cpu_count = 16;
    data.total_memory = 64ULL << 30;
    data.cpu_load_basis_points = 2500;
    data.available_memory = 48ULL << 30;
    data.running_executions = 3;

    const std::size_t capability_count = large ? 64 : 8;
    const std::size_t device_count = large ? 16 : 2;
    for (std::size_t index = 0; index < capability_count; ++index) {
        data.capabilities.push_back(padded("gpu.capability-", index, large));
    }
    for (std::size_t index = 0; index < device_count; ++index) {
        data.devices.push_back(padded("device.sensor-", index, large));
    }
    return data;
}

template <class Encode, class Decode>
void measure_case(
    std::string_view codec,
    std::string_view message,
    std::string_view profile,
    std::size_t bytes,
    int iterations,
    Encode encode,
    Decode decode) {
    for (int index = 0; index < 16; ++index) {
        g_size_sink = encode();
        g_checksum_sink ^= decode();
    }

    std::vector<double> encode_samples;
    std::vector<double> decode_samples;
    encode_samples.reserve(static_cast<std::size_t>(iterations));
    decode_samples.reserve(static_cast<std::size_t>(iterations));

    for (int index = 0; index < iterations; ++index) {
        const auto start = Clock::now();
        g_size_sink = encode();
        const auto end = Clock::now();
        encode_samples.push_back(
            std::chrono::duration<double, std::nano>(end - start).count());
    }
    for (int index = 0; index < iterations; ++index) {
        const auto start = Clock::now();
        g_checksum_sink ^= decode();
        const auto end = Clock::now();
        decode_samples.push_back(
            std::chrono::duration<double, std::nano>(end - start).count());
    }

    auto percentile = [](std::vector<double> samples, double fraction) {
        std::sort(samples.begin(), samples.end());
        const auto index = static_cast<std::size_t>(
            fraction * static_cast<double>(samples.size() - 1));
        return samples[index];
    };

    std::cout << "codec=" << codec << " message=" << message
              << " profile=" << profile << " bytes=" << bytes
              << " encode_p50_ns=" << percentile(encode_samples, 0.50)
              << " encode_p95_ns=" << percentile(encode_samples, 0.95)
              << " decode_p50_ns=" << percentile(decode_samples, 0.50)
              << " decode_p95_ns=" << percentile(decode_samples, 0.95)
              << " iterations=" << iterations << '\n';
}

// --- Protocol Buffers -------------------------------------------------------

pb::Submit to_protobuf(const SubmitData& data) {
    pb::Submit message;
    message.set_request_id(data.request_id);
    message.set_executable(data.executable);
    for (const auto& argument : data.arguments) {
        message.add_arguments(argument);
    }
    for (const auto& value : data.environment) {
        (*message.mutable_environment())[value.key] = value.value;
    }
    for (const auto& value : data.labels) {
        (*message.mutable_labels())[value.key] = value.value;
    }
    for (const auto& value : data.inputs) {
        auto* input = message.add_inputs();
        input->set_artifact(value.artifact);
        input->set_path(value.path);
        input->set_access(value.read_write ? pb::ACCESS_READ_WRITE : pb::ACCESS_READ_ONLY);
    }
    for (const auto& output : data.outputs) {
        message.add_outputs(output);
    }
    message.set_lifetime(data.lifetime == 0 ? pb::LIFETIME_FINITE : pb::LIFETIME_PERSISTENT);
    for (const auto& service : data.services) {
        message.add_services(service);
    }
    message.set_cpu_millicores(data.cpu_millicores);
    message.set_memory_bytes(data.memory_bytes);
    message.set_architecture(data.architecture);
    message.set_abi(data.abi);
    for (const auto& value : data.required_capabilities) {
        message.add_required_capabilities(value);
    }
    for (const auto& value : data.required_devices) {
        message.add_required_devices(value);
    }
    message.set_explicit_node(data.explicit_node);
    return message;
}

std::vector<std::byte> encode_protobuf_submit(const SubmitData& data) {
    const auto message = to_protobuf(data);
    std::vector<std::byte> buffer(message.ByteSizeLong());
    if (!message.SerializeToArray(buffer.data(), static_cast<int>(buffer.size()))) {
        throw std::runtime_error("protobuf submit serialization failed");
    }
    return buffer;
}

std::uint64_t decode_protobuf_submit(std::span<const std::byte> bytes) {
    pb::Submit message;
    if (!message.ParseFromArray(bytes.data(), static_cast<int>(bytes.size()))) {
        throw std::runtime_error("protobuf submit parse failed");
    }
    return static_cast<std::uint64_t>(message.arguments_size()) +
           static_cast<std::uint64_t>(message.environment_size()) +
           static_cast<std::uint64_t>(message.labels_size()) +
           static_cast<std::uint64_t>(message.inputs_size()) +
           static_cast<std::uint64_t>(message.outputs_size()) +
           static_cast<std::uint64_t>(message.services_size()) +
           static_cast<std::uint64_t>(message.required_capabilities_size()) +
           static_cast<std::uint64_t>(message.required_devices_size()) +
           static_cast<std::uint64_t>(message.request_id().size()) +
           static_cast<std::uint64_t>(message.cpu_millicores()) +
           message.memory_bytes();
}

pb::NodeDescriptor to_protobuf(const NodeDescriptorData& data) {
    pb::NodeDescriptor message;
    message.set_id(data.id);
    message.set_name(data.name);
    message.set_abi(data.abi);
    message.set_architecture(data.architecture);
    message.set_logical_cpu_count(data.logical_cpu_count);
    message.set_total_memory(data.total_memory);
    for (const auto& value : data.capabilities) {
        message.add_capabilities(value);
    }
    for (const auto& value : data.devices) {
        message.add_devices(value);
    }
    message.set_cpu_load_basis_points(data.cpu_load_basis_points);
    message.set_available_memory(data.available_memory);
    message.set_running_executions(data.running_executions);
    return message;
}

std::vector<std::byte> encode_protobuf_node(const NodeDescriptorData& data) {
    const auto message = to_protobuf(data);
    std::vector<std::byte> buffer(message.ByteSizeLong());
    if (!message.SerializeToArray(buffer.data(), static_cast<int>(buffer.size()))) {
        throw std::runtime_error("protobuf node serialization failed");
    }
    return buffer;
}

std::uint64_t decode_protobuf_node(std::span<const std::byte> bytes) {
    pb::NodeDescriptor message;
    if (!message.ParseFromArray(bytes.data(), static_cast<int>(bytes.size()))) {
        throw std::runtime_error("protobuf node parse failed");
    }
    return static_cast<std::uint64_t>(message.capabilities_size()) +
           static_cast<std::uint64_t>(message.devices_size()) +
           static_cast<std::uint64_t>(message.name().size()) +
           message.total_memory() + message.available_memory() +
           message.logical_cpu_count() + message.running_executions();
}

// --- FlatBuffers ------------------------------------------------------------

std::vector<std::byte> encode_flatbuffers_submit(const SubmitData& data) {
    flatbuffers::FlatBufferBuilder builder(1024);
    const auto request_id = builder.CreateString(data.request_id);
    const auto executable = builder.CreateString(data.executable);

    std::vector<flatbuffers::Offset<flatbuffers::String>> arguments;
    arguments.reserve(data.arguments.size());
    for (const auto& value : data.arguments) {
        arguments.push_back(builder.CreateString(value));
    }
    std::vector<flatbuffers::Offset<fb::KeyValue>> environment;
    environment.reserve(data.environment.size());
    for (const auto& value : data.environment) {
        environment.push_back(fb::CreateKeyValue(
            builder, builder.CreateString(value.key), builder.CreateString(value.value)));
    }
    std::vector<flatbuffers::Offset<fb::KeyValue>> labels;
    labels.reserve(data.labels.size());
    for (const auto& value : data.labels) {
        labels.push_back(fb::CreateKeyValue(
            builder, builder.CreateString(value.key), builder.CreateString(value.value)));
    }
    std::vector<flatbuffers::Offset<fb::ArtifactInput>> inputs;
    inputs.reserve(data.inputs.size());
    for (const auto& value : data.inputs) {
        inputs.push_back(fb::CreateArtifactInput(
            builder,
            builder.CreateString(value.artifact),
            builder.CreateString(value.path),
            value.read_write ? fb::Access_read_write : fb::Access_read_only));
    }
    std::vector<flatbuffers::Offset<flatbuffers::String>> outputs;
    outputs.reserve(data.outputs.size());
    for (const auto& value : data.outputs) {
        outputs.push_back(builder.CreateString(value));
    }
    std::vector<flatbuffers::Offset<flatbuffers::String>> services;
    services.reserve(data.services.size());
    for (const auto& value : data.services) {
        services.push_back(builder.CreateString(value));
    }
    std::vector<flatbuffers::Offset<flatbuffers::String>> capabilities;
    capabilities.reserve(data.required_capabilities.size());
    for (const auto& value : data.required_capabilities) {
        capabilities.push_back(builder.CreateString(value));
    }
    std::vector<flatbuffers::Offset<flatbuffers::String>> devices;
    devices.reserve(data.required_devices.size());
    for (const auto& value : data.required_devices) {
        devices.push_back(builder.CreateString(value));
    }
    const auto explicit_node = builder.CreateString(data.explicit_node);

    fb::SubmitBuilder submit(builder);
    submit.add_request_id(request_id);
    submit.add_executable(executable);
    submit.add_arguments(builder.CreateVector(arguments));
    submit.add_environment(builder.CreateVector(environment));
    submit.add_labels(builder.CreateVector(labels));
    submit.add_inputs(builder.CreateVector(inputs));
    submit.add_outputs(builder.CreateVector(outputs));
    submit.add_lifetime(
        data.lifetime == 0 ? fb::Lifetime_finite : fb::Lifetime_persistent);
    submit.add_services(builder.CreateVector(services));
    submit.add_cpu_millicores(data.cpu_millicores);
    submit.add_memory_bytes(data.memory_bytes);
    submit.add_architecture(builder.CreateString(data.architecture));
    submit.add_abi(builder.CreateString(data.abi));
    submit.add_required_capabilities(builder.CreateVector(capabilities));
    submit.add_required_devices(builder.CreateVector(devices));
    submit.add_explicit_node(explicit_node);
    builder.Finish(submit.Finish());

    const auto* start = builder.GetBufferPointer();
    std::vector<std::byte> bytes(builder.GetSize());
    std::memcpy(bytes.data(), start, bytes.size());
    return bytes;
}

std::uint64_t decode_flatbuffers_submit(std::span<const std::byte> bytes) {
    flatbuffers::Verifier verifier(
        reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
    if (!fb::VerifySubmitBuffer(verifier)) {
        throw std::runtime_error("flatbuffers submit verification failed");
    }
    const auto* message = fb::GetSubmit(bytes.data());
    return static_cast<std::uint64_t>(message->arguments()->size()) +
           static_cast<std::uint64_t>(message->environment()->size()) +
           static_cast<std::uint64_t>(message->labels()->size()) +
           static_cast<std::uint64_t>(message->inputs()->size()) +
           static_cast<std::uint64_t>(message->outputs()->size()) +
           static_cast<std::uint64_t>(message->services()->size()) +
           static_cast<std::uint64_t>(message->required_capabilities()->size()) +
           static_cast<std::uint64_t>(message->required_devices()->size()) +
           static_cast<std::uint64_t>(message->request_id()->size()) +
           static_cast<std::uint64_t>(message->cpu_millicores()) +
           message->memory_bytes();
}

std::vector<std::byte> encode_flatbuffers_node(const NodeDescriptorData& data) {
    flatbuffers::FlatBufferBuilder builder(512);
    std::vector<flatbuffers::Offset<flatbuffers::String>> capabilities;
    capabilities.reserve(data.capabilities.size());
    for (const auto& value : data.capabilities) {
        capabilities.push_back(builder.CreateString(value));
    }
    std::vector<flatbuffers::Offset<flatbuffers::String>> devices;
    devices.reserve(data.devices.size());
    for (const auto& value : data.devices) {
        devices.push_back(builder.CreateString(value));
    }

    fb::NodeDescriptorBuilder node(builder);
    node.add_id(builder.CreateString(data.id));
    node.add_name(builder.CreateString(data.name));
    node.add_abi(builder.CreateString(data.abi));
    node.add_architecture(builder.CreateString(data.architecture));
    node.add_logical_cpu_count(data.logical_cpu_count);
    node.add_total_memory(data.total_memory);
    node.add_capabilities(builder.CreateVector(capabilities));
    node.add_devices(builder.CreateVector(devices));
    node.add_cpu_load_basis_points(data.cpu_load_basis_points);
    node.add_available_memory(data.available_memory);
    node.add_running_executions(data.running_executions);
    builder.Finish(node.Finish());

    const auto* start = builder.GetBufferPointer();
    std::vector<std::byte> bytes(builder.GetSize());
    std::memcpy(bytes.data(), start, bytes.size());
    return bytes;
}

std::uint64_t decode_flatbuffers_node(std::span<const std::byte> bytes) {
    flatbuffers::Verifier verifier(
        reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size());
    if (!fb::VerifyNodeDescriptorBuffer(verifier)) {
        throw std::runtime_error("flatbuffers node verification failed");
    }
    const auto* message = fb::GetNodeDescriptor(bytes.data());
    return static_cast<std::uint64_t>(message->capabilities()->size()) +
           static_cast<std::uint64_t>(message->devices()->size()) +
           static_cast<std::uint64_t>(message->name()->size()) +
           message->total_memory() + message->available_memory() +
           message->logical_cpu_count() + message->running_executions();
}

// --- Cap'n Proto ------------------------------------------------------------

std::vector<::capnp::word> encode_capnp_submit(const SubmitData& data) {
    ::capnp::MallocMessageBuilder builder;
    auto message = builder.initRoot<capnp::Submit>();
    message.setRequestId(kj::StringPtr(data.request_id));
    message.setExecutable(kj::StringPtr(data.executable));

    auto arguments = message.initArguments(data.arguments.size());
    for (std::size_t index = 0; index < data.arguments.size(); ++index) {
        arguments.set(index, kj::StringPtr(data.arguments[index]));
    }
    auto environment = message.initEnvironment(data.environment.size());
    for (std::size_t index = 0; index < data.environment.size(); ++index) {
        auto value = environment[index];
        value.setKey(kj::StringPtr(data.environment[index].key));
        value.setValue(kj::StringPtr(data.environment[index].value));
    }
    auto labels = message.initLabels(data.labels.size());
    for (std::size_t index = 0; index < data.labels.size(); ++index) {
        auto value = labels[index];
        value.setKey(kj::StringPtr(data.labels[index].key));
        value.setValue(kj::StringPtr(data.labels[index].value));
    }
    auto inputs = message.initInputs(data.inputs.size());
    for (std::size_t index = 0; index < data.inputs.size(); ++index) {
        auto value = inputs[index];
        value.setArtifact(kj::StringPtr(data.inputs[index].artifact));
        value.setPath(kj::StringPtr(data.inputs[index].path));
        value.setAccess(
            data.inputs[index].read_write ? capnp::Access::READ_WRITE
                                           : capnp::Access::READ_ONLY);
    }
    auto outputs = message.initOutputs(data.outputs.size());
    for (std::size_t index = 0; index < data.outputs.size(); ++index) {
        outputs.set(index, kj::StringPtr(data.outputs[index]));
    }
    message.setLifetime(
        data.lifetime == 0 ? capnp::Lifetime::FINITE : capnp::Lifetime::PERSISTENT);
    auto services = message.initServices(data.services.size());
    for (std::size_t index = 0; index < data.services.size(); ++index) {
        services.set(index, kj::StringPtr(data.services[index]));
    }
    message.setCpuMillicores(data.cpu_millicores);
    message.setMemoryBytes(data.memory_bytes);
    message.setArchitecture(kj::StringPtr(data.architecture));
    message.setAbi(kj::StringPtr(data.abi));
    auto capabilities = message.initRequiredCapabilities(data.required_capabilities.size());
    for (std::size_t index = 0; index < data.required_capabilities.size(); ++index) {
        capabilities.set(index, kj::StringPtr(data.required_capabilities[index]));
    }
    auto devices = message.initRequiredDevices(data.required_devices.size());
    for (std::size_t index = 0; index < data.required_devices.size(); ++index) {
        devices.set(index, kj::StringPtr(data.required_devices[index]));
    }
    message.setExplicitNode(kj::StringPtr(data.explicit_node));

    const auto words = ::capnp::messageToFlatArray(builder);
    return std::vector<::capnp::word>(words.begin(), words.end());
}

std::uint64_t decode_capnp_submit(std::span<const ::capnp::word> words) {
    ::capnp::FlatArrayMessageReader reader(kj::arrayPtr(words.data(), words.size()));
    const auto message = reader.getRoot<capnp::Submit>();
    return static_cast<std::uint64_t>(message.getArguments().size()) +
           static_cast<std::uint64_t>(message.getEnvironment().size()) +
           static_cast<std::uint64_t>(message.getLabels().size()) +
           static_cast<std::uint64_t>(message.getInputs().size()) +
           static_cast<std::uint64_t>(message.getOutputs().size()) +
           static_cast<std::uint64_t>(message.getServices().size()) +
           static_cast<std::uint64_t>(message.getRequiredCapabilities().size()) +
           static_cast<std::uint64_t>(message.getRequiredDevices().size()) +
           static_cast<std::uint64_t>(message.getRequestId().size()) +
           static_cast<std::uint64_t>(message.getCpuMillicores()) +
           message.getMemoryBytes();
}

std::vector<::capnp::word> encode_capnp_node(const NodeDescriptorData& data) {
    ::capnp::MallocMessageBuilder builder;
    auto message = builder.initRoot<capnp::NodeDescriptor>();
    message.setId(kj::StringPtr(data.id));
    message.setName(kj::StringPtr(data.name));
    message.setAbi(kj::StringPtr(data.abi));
    message.setArchitecture(kj::StringPtr(data.architecture));
    message.setLogicalCpuCount(data.logical_cpu_count);
    message.setTotalMemory(data.total_memory);
    auto capabilities = message.initCapabilities(data.capabilities.size());
    for (std::size_t index = 0; index < data.capabilities.size(); ++index) {
        capabilities.set(index, kj::StringPtr(data.capabilities[index]));
    }
    auto devices = message.initDevices(data.devices.size());
    for (std::size_t index = 0; index < data.devices.size(); ++index) {
        devices.set(index, kj::StringPtr(data.devices[index]));
    }
    message.setCpuLoadBasisPoints(data.cpu_load_basis_points);
    message.setAvailableMemory(data.available_memory);
    message.setRunningExecutions(data.running_executions);

    const auto words = ::capnp::messageToFlatArray(builder);
    return std::vector<::capnp::word>(words.begin(), words.end());
}

std::uint64_t decode_capnp_node(std::span<const ::capnp::word> words) {
    ::capnp::FlatArrayMessageReader reader(kj::arrayPtr(words.data(), words.size()));
    const auto message = reader.getRoot<capnp::NodeDescriptor>();
    return static_cast<std::uint64_t>(message.getCapabilities().size()) +
           static_cast<std::uint64_t>(message.getDevices().size()) +
           static_cast<std::uint64_t>(message.getName().size()) +
           message.getTotalMemory() + message.getAvailableMemory() +
           message.getLogicalCpuCount() + message.getRunningExecutions();
}

void run_submit_case(std::string_view codec, std::string_view profile, bool large) {
    const SubmitData data = make_submit(large);
    const int iterations = large ? 1000 : 3000;

    if (codec == "protobuf") {
        const auto encoded = encode_protobuf_submit(data);
        measure_case(
            codec, "submit", profile, encoded.size(), iterations,
            [&] { return encode_protobuf_submit(data).size(); },
            [&] { return decode_protobuf_submit(encoded); });
    } else if (codec == "flatbuffers") {
        const auto encoded = encode_flatbuffers_submit(data);
        measure_case(
            codec, "submit", profile, encoded.size(), iterations,
            [&] { return encode_flatbuffers_submit(data).size(); },
            [&] { return decode_flatbuffers_submit(encoded); });
    } else {
        const auto encoded = encode_capnp_submit(data);
        const auto size = encoded.size() * sizeof(::capnp::word);
        measure_case(
            codec, "submit", profile, size, iterations,
            [&] { return encode_capnp_submit(data).size() * sizeof(::capnp::word); },
            [&] { return decode_capnp_submit(encoded); });
    }
}

void run_node_case(std::string_view codec, std::string_view profile, bool large) {
    const NodeDescriptorData data = make_node(large);
    const int iterations = large ? 1000 : 3000;

    if (codec == "protobuf") {
        const auto encoded = encode_protobuf_node(data);
        measure_case(
            codec, "node_descriptor", profile, encoded.size(), iterations,
            [&] { return encode_protobuf_node(data).size(); },
            [&] { return decode_protobuf_node(encoded); });
    } else if (codec == "flatbuffers") {
        const auto encoded = encode_flatbuffers_node(data);
        measure_case(
            codec, "node_descriptor", profile, encoded.size(), iterations,
            [&] { return encode_flatbuffers_node(data).size(); },
            [&] { return decode_flatbuffers_node(encoded); });
    } else {
        const auto encoded = encode_capnp_node(data);
        const auto size = encoded.size() * sizeof(::capnp::word);
        measure_case(
            codec, "node_descriptor", profile, size, iterations,
            [&] { return encode_capnp_node(data).size() * sizeof(::capnp::word); },
            [&] { return decode_capnp_node(encoded); });
    }
}

} // namespace

void run_benchmark() {
    for (const std::string_view codec : {"protobuf", "flatbuffers", "capnproto"}) {
        run_submit_case(codec, "typical", false);
        run_submit_case(codec, "large", true);
        run_node_case(codec, "typical", false);
        run_node_case(codec, "large", true);
    }
}

} // namespace pluribus::bench

int main() {
    try {
        pluribus::bench::run_benchmark();
    } catch (const std::exception& exception) {
        std::cerr << "benchmark failed: " << exception.what() << '\n';
        return 1;
    }
    return 0;
}
