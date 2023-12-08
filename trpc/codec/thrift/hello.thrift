namespace cpp trpc.test.helloworld;

struct HelloRequest {
    1: required string name;
}

struct HelloReply {
    1: required string message;
}

service Greeter {
  HelloReply SayHello(1:HelloRequest req);
}