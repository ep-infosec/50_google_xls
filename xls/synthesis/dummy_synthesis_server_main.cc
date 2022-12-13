// Copyright 2020 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"
#include "grpcpp/server_context.h"
#include "xls/common/init_xls.h"
#include "xls/common/logging/logging.h"
#include "xls/common/status/status_macros.h"
#include "xls/synthesis/server_credentials.h"
#include "xls/synthesis/synthesis.pb.h"
#include "xls/synthesis/synthesis_service.grpc.pb.h"

const char kUsage[] = R"(
Launches a XLS synthesis server which serves dummy results. The flag
--max_frequency_ghz is used to determine whether a negative slack value is
returned in the response.

Invocation:

  dummy_synthesis_server --max_frequency_ghz=4.2
)";

ABSL_FLAG(int32_t, port, 10000, "Port to listen on.");
ABSL_FLAG(double, max_frequency_ghz, 1000,
          "The maximum frequency to use for any synthesis request.");
ABSL_FLAG(bool, serve_errors, false,
          "Whether to serve an error in the response.");

namespace xls {
namespace synthesis {
namespace {

// Service implementation that dispatches compile requests.
class DummySynthesisServiceImpl : public SynthesisService::Service {
 public:
  explicit DummySynthesisServiceImpl(int64_t max_frequency_hz,
                                     bool serve_errors)
      : max_frequency_hz_(max_frequency_hz), serve_errors_(serve_errors) {}

  ::grpc::Status Compile(::grpc::ServerContext* server_context,
                         const CompileRequest* request,
                         CompileResponse* result) override {
    auto start = absl::Now();

    result->set_slack_ps(request->target_frequency_hz() <= max_frequency_hz_
                             ? 0
                             : 1e12L / request->target_frequency_hz() -
                                   1e12L / max_frequency_hz_);
    result->set_power(42);
    result->set_area(123);
    result->set_netlist("// NETLIST");
    result->set_elapsed_runtime_ms(
        absl::ToInt64Milliseconds(absl::Now() - start));
    if (serve_errors_) {
      return ::grpc::Status(grpc::StatusCode::INTERNAL,
                            "Dummy synthesis server error");
    }
    return ::grpc::Status::OK;
  }

 private:
  int64_t max_frequency_hz_;
  bool serve_errors_;
};

void RealMain() {
  int64_t max_frequency_hz =
      static_cast<int64_t>(1e9 * absl::GetFlag(FLAGS_max_frequency_ghz));
  int port = absl::GetFlag(FLAGS_port);
  std::string server_address = absl::StrCat("0.0.0.0:", port);
  DummySynthesisServiceImpl service(max_frequency_hz,
                                    absl::GetFlag(FLAGS_serve_errors));

  ::grpc::ServerBuilder builder;
  std::shared_ptr<::grpc::ServerCredentials> creds = GetServerCredentials();
  builder.AddListeningPort(server_address, creds);
  builder.RegisterService(&service);
  std::unique_ptr<::grpc::Server> server(builder.BuildAndStart());
  XLS_LOG(INFO) << "Serving on port: " << port;
  server->Wait();
}

}  // namespace
}  // namespace synthesis
}  // namespace xls

int main(int argc, char** argv) {
  xls::InitXls(kUsage, argc, argv);

  xls::synthesis::RealMain();

  return EXIT_SUCCESS;
}
