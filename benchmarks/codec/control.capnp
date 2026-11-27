@0x9f3a1c2d4e5b6071;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("pluribus::bench::capnp");

enum Lifetime {
  finite @0;
  persistent @1;
}

enum Access {
  readOnly @0;
  readWrite @1;
}

struct ArtifactInput {
  artifact @0 :Text;
  path @1 :Text;
  access @2 :Access;
}

struct KeyValue {
  key @0 :Text;
  value @1 :Text;
}

struct Submit {
  requestId @0 :Text;
  executable @1 :Text;
  arguments @2 :List(Text);
  environment @3 :List(KeyValue);
  labels @4 :List(KeyValue);
  inputs @5 :List(ArtifactInput);
  outputs @6 :List(Text);
  lifetime @7 :Lifetime;
  services @8 :List(Text);
  cpuMillicores @9 :Int64;
  memoryBytes @10 :UInt64;
  architecture @11 :Text;
  abi @12 :Text;
  requiredCapabilities @13 :List(Text);
  requiredDevices @14 :List(Text);
  explicitNode @15 :Text;
}

struct NodeDescriptor {
  id @0 :Text;
  name @1 :Text;
  abi @2 :Text;
  architecture @3 :Text;
  logicalCpuCount @4 :UInt32;
  totalMemory @5 :UInt64;
  capabilities @6 :List(Text);
  devices @7 :List(Text);
  cpuLoadBasisPoints @8 :UInt32;
  availableMemory @9 :UInt64;
  runningExecutions @10 :UInt32;
}
