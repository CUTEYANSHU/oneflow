#ifndef ONEFLOW_XRT_BUILD_GRAPH_H_
#define ONEFLOW_XRT_BUILD_GRAPH_H_

#include "oneflow/core/graph/op_graph.h"
#include "oneflow/core/job/job_desc.h"
#include "oneflow/core/operator/op_conf.pb.h"
#include "oneflow/xrt/api.h"
#include "oneflow/xrt/graph/graph.h"

namespace oneflow {
namespace xrt {

extern const std::string _ArgumentOpType;

namespace graph_builder {

class GraphBuilder {
 public:
  GraphBuilder() = delete;

  explicit GraphBuilder(const OpGraph *op_graph);

  explicit GraphBuilder(const XrtLaunchOpConf::Function &function,
                        const DeviceType &device_type, const JobDesc &job_desc);

  std::shared_ptr<XrtGraph> Build() {
    BuildGraphEdges();
    SetupGraphEdges();
    return graph_;
  }

  struct NodeInfo {
    util::Set<std::string> inputs;
    util::Map<std::string, std::string> input_output_keys;
    const OpNode *op_node = nullptr;
  };

 private:
  void SetupXrtNode(XrtNode *node, const OperatorConf &node_conf) const {
    node->set_name(node_conf.name());
    node->set_type(ExtractOpTypeAsString(node_conf));
    node->set_device(DeviceTypeToXrtDevice(node_conf.device_type()));
  }

  void SetupXrtNode(XrtNode *node,
                    const XrtLaunchOpConf::Argument &arg_conf) const {
    node->set_name(arg_conf.name());
    node->set_type(_ArgumentOpType);
    node->set_device(DeviceTypeToXrtDevice(arg_conf.device_type()));
  }

  void MakeMetaData(const XrtNode *start, const XrtNode *end,
                    const std::string &arg_name, ArgumentMetaData *meta_data);

  void BuildGraphEdges();
  void SetupGraphEdges();

 private:
  std::shared_ptr<XrtGraph> graph_;
  util::Map<std::string, const XrtNode *> producers_;
  util::Map<const XrtNode *, NodeInfo> node_info_;
};

std::shared_ptr<XrtGraph> BuildGraph(const XrtLaunchOpConf::Function &function,
                                     const DeviceType &device_type,
                                     const JobDesc &job_desc);

std::shared_ptr<XrtGraph> BuildGraph(const OpGraph *op_graph);

}  // namespace graph_builder

}  // namespace xrt
}  // namespace oneflow

#endif  // ONEFLOW_XRT_BUILD_GRAPH_H_