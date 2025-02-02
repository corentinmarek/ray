#include "native_task_submitter.h"

#include <ray/api/ray_exception.h>

#include "../abstract_ray_runtime.h"

namespace ray {
namespace api {

RayFunction BuildRayFunction(InvocationSpec &invocation) {
  auto function_descriptor = FunctionDescriptorBuilder::BuildCpp(
      invocation.lib_name, invocation.remote_function_holder.function_name);
  return RayFunction(Language::CPP, function_descriptor);
}

ObjectID NativeTaskSubmitter::Submit(InvocationSpec &invocation) {
  auto &core_worker = CoreWorkerProcess::GetCoreWorker();
  std::vector<ObjectID> return_ids;
  if (invocation.task_type == TaskType::ACTOR_TASK) {
    core_worker.SubmitActorTask(invocation.actor_id, BuildRayFunction(invocation),
                                invocation.args, TaskOptions(), &return_ids);
  } else {
    core_worker.SubmitTask(BuildRayFunction(invocation), invocation.args, TaskOptions(),
                           &return_ids, 1, std::make_pair(PlacementGroupID::Nil(), -1),
                           true, "");
  }
  return return_ids[0];
}

ObjectID NativeTaskSubmitter::SubmitTask(InvocationSpec &invocation) {
  return Submit(invocation);
}

ActorID NativeTaskSubmitter::CreateActor(InvocationSpec &invocation) {
  auto &core_worker = CoreWorkerProcess::GetCoreWorker();

  std::unordered_map<std::string, double> resources;
  std::string name = "";
  ActorCreationOptions actor_options{0,
                                     0,
                                     1,
                                     resources,
                                     resources,
                                     {},
                                     /*is_detached=*/false,
                                     name,
                                     /*is_asyncio=*/false};
  ActorID actor_id;
  auto status = core_worker.CreateActor(BuildRayFunction(invocation), invocation.args,
                                        actor_options, "", &actor_id);
  if (!status.ok()) {
    throw RayException("Create actor error");
  }
  return actor_id;
}

ObjectID NativeTaskSubmitter::SubmitActorTask(InvocationSpec &invocation) {
  return Submit(invocation);
}

}  // namespace api
}  // namespace ray
