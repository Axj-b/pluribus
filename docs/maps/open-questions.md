---
type: map
id: open-questions-map
status: active
tags: [map, question]
aliases: ["Open issues MOC"]
last_reviewed: 2026-11-25
---

# Open questions map

> A theme view of the [open questions register](../questions/README.md). The register is canonical
> for status and next action; this map is for navigation.

## Execution and lifecycle

- [OI-01 ExecutionId representation](../questions/README.md#oi-01-executionid-representation) — wire identity, friendly IDs, lookup ambiguity.
- [OI-02 Owner identity and lifetime](../questions/README.md#oi-02-owner-identity-and-lifetime) — detach and owner coupling.
- [OI-03 Orphan policy and retention](../questions/README.md#oi-03-orphan-policy-and-retention) — unknown/stale process handling.
- [OI-05 Exit status and output collection](../questions/README.md#oi-05-exit-status-and-output-collection) — portable outcomes and collection failures.

## Protocol, fabric and wire

- [OI-06 Control message schema and type registry](../questions/README.md#oi-06-control-message-schema-and-type-registry) — payload schemas and numeric IDs.
- [OI-07 Endpoint tickets and route authorization](../questions/README.md#oi-07-endpoint-tickets-and-route-authorization) — service routing capabilities.
- [OI-09 Request deduplication and durable master state](../questions/README.md#oi-09-request-deduplication-and-durable-master-state) — retry safety.
- [OI-14 Serializer selection](../questions/README.md#oi-14-serializer-selection) — codec benchmark and choice.
- [OI-15 Data-plane framing and limits](../questions/README.md#oi-15-data-plane-framing-and-limits) — bulk transfer bounds.
- [OI-16 Wire compatibility and conformance](../questions/README.md#oi-16-wire-compatibility-and-conformance) — evolution tests.
- [OI-25 Network stack selection](../questions/README.md#oi-25-network-stack-selection) — TCP/async stack.

## Identity, authorization and security

- [OI-08 Authentication and client identity](../questions/README.md#oi-08-authentication-and-client-identity) — join token vs. real auth.
- [OI-23 Client authorization and local isolation](../questions/README.md#oi-23-client-authorization-and-local-isolation) — host-dependent local security.
- [OI-24 Hard isolation threat model](../questions/README.md#oi-24-hard-isolation-threat-model) — untrusted workloads.

## API, services and runtime

- [OI-10 Public API ownership and async model](../questions/README.md#oi-10-public-api-ownership-and-async-model) — handles, cancellation, threading.
- [OI-11 Log cursor and stream semantics](../questions/README.md#oi-11-log-cursor-and-stream-semantics) — replay gaps and shutdown.
- [OI-12 Service-name grammar](../questions/README.md#oi-12-service-name-grammar) — interoperability grammar.
- [OI-13 Duplicate provider and readiness policy](../questions/README.md#oi-13-duplicate-provider-and-readiness-policy) — multiple providers, one resolution.
- [OI-17 Host backend signatures and environment](../questions/README.md#oi-17-host-backend-signatures-and-environment) — backend and `PLURIBUS_*` bridge.

## Scheduling

- [OI-18 Scheduler defaults and measurement freshness](../questions/README.md#oi-18-scheduler-defaults-and-measurement-freshness) — deployment policy, not wire constants.

## Store, packaging and ABI

- [OI-04 Portable native ABI](../questions/README.md#oi-04-portable-native-abi) — the runtime/loader contract.
- [OI-19 Artifact manifest and package resolution](../questions/README.md#oi-19-artifact-manifest-and-package-resolution) — canonical encoding and leases.
- [OI-20 Cross-host ABI package mapping](../questions/README.md#oi-20-cross-host-abi-package-mapping) — variant selection without host OS.
- [OI-26 Workspace snapshot adoption](../questions/README.md#oi-26-workspace-snapshot-adoption) — proposed content-addressed cache.
- [OI-28 Minimal hello resolution](../questions/README.md#oi-28-minimal-hello-resolution) — first vertical slice.

## Engineering and operations

- [OI-21 License and platform validation](../questions/README.md#oi-21-license-and-platform-validation) — publication and DRIVE OS.
- [OI-22 Dependency enforcement and toolchain pinning](../questions/README.md#oi-22-dependency-enforcement-and-toolchain-pinning) — make boundaries real.
- [OI-27 Test and dependency baseline](../questions/README.md#oi-27-test-and-dependency-baseline) — vcpkg/GoogleTest decision.

## Related

- [Open questions register](../questions/README.md)
- [Decision map](decisions.md)
- [Implementation map](implementation.md)
- [Template](../templates/question.md)
