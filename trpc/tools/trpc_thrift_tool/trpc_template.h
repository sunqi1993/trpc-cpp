#pragma once

#include <string>
#include <string_view>


const std::string_view trpc_source_header_fmt=R"(
#include "%s"
#include "trpc/server/rpc_async_method_handler.h"
#include "trpc/server/rpc_method_handler.h"
)";


const std::string_view trpc_service_sync_method_fmt=R"(
virtual ::trpc::Status %s(%s context, const %s request, %s* response);
)";

const std::string_view trpc_service_async_method_fmt=R"(
virtual ::trpc::Future<%s> %s(%s context, const %s request);
)";


const std::string_view trpc_service_registered_methods=R"(
const std::vector<std::string_view> %s_method_names={%s};
)";


// 服务端初始化策略
const std::string_view trpc_service_server_ctor_reg_method_fmt=R"(
AddRpcServiceMethod(new ::trpc::RpcServiceMethod(%s, ::trpc::MethodType::UNARY, new ::trpc::%sRpcMethodHandler<%s, %s>(std::bind(&%s::%s, this, std::placeholders::_1, std::placeholders::_2%s))));
)";

const std::string_view trpc_service_server_sync_method_impl=R"(
::trpc::Status %s::%s(::trpc::ServerContextPtr context, const %s* request, %s* response) {
  (void)context;
  (void)request;
  (void)response;
  return ::trpc::Status(-1, "");
}
)";

const std::string_view trpc_service_server_async_method_impl=R"(
::trpc::Future<%s> %s::%s(const ::trpc::ServerContextPtr& context, const %s* request) {
  return ::trpc::MakeExceptionFuture<%s>(::trpc::CommonException("Unimplemented"));
}
)";

const std::string_view trpc_service_client_method_impl=R"(
::trpc::Status %s::%s(const ::trpc::ClientContextPtr& context, const %s& request, %s* response) {
  if (context->GetFuncName().empty()) context->SetFuncName(%s);
  return UnaryInvoke<%s, %s>(context, request, response);
}
)";