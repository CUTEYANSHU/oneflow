#include "oneflow/core/operator/decode_ofrecord_op.h"
#include "oneflow/core/record/ofrecord_decoder.h"

namespace oneflow {

void DecodeOFRecordOp::InitFromOpConf() {
  CHECK(op_conf().has_decode_ofrecord_conf());
  EnrollInputBn("in", false);
  const DecodeOFRecordOpConf& conf = op_conf().decode_ofrecord_conf();
  for (int32_t i = 0; i < conf.blob_size(); ++i) {
    if (IsPbDataType(conf.blob(i).data_type())) {
      EnrollPbOutputBn("out_" + std::to_string(i));
    } else {
      EnrollOutputBn("out_" + std::to_string(i), false);
    }
  }
  if (conf.part_name_suffix_length() != -1) {
    CHECK_GE(conf.part_name_suffix_length(),
             std::to_string(Global<JobDesc>::Get()->other_conf().data_part_num() - 1).length());
  }
}

void DecodeOFRecordOp::VirtualGenKernelConf(
    std::function<const BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx, KernelConf* kernel_conf) const {
  kernel_conf->mutable_decode_ofrecord_conf()->set_random_seed(NewRandomSeed());
}

const PbMessage& DecodeOFRecordOp::GetCustomizedConf() const {
  return op_conf().decode_ofrecord_conf();
}

void DecodeOFRecordOp::InferBlobDescs(
    std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
    const ParallelContext* parallel_ctx) const {
  BlobDesc* in_blob_desc = GetBlobDesc4BnInOp(SoleIbn());
  FOR_RANGE(size_t, i, 0, output_bns().size() + pb_output_bns().size()) {
    BlobDesc* out_blob_desc = GetBlobDesc4BnInOp("out_" + std::to_string(i));
    const BlobConf& blob_conf = op_conf().decode_ofrecord_conf().blob(i);
    std::vector<int64_t> dim_vec(1 + blob_conf.shape().dim_size());
    dim_vec[0] = in_blob_desc->shape().At(0);
    FOR_RANGE(size_t, j, 1, dim_vec.size()) { dim_vec[j] = blob_conf.shape().dim(j - 1); }
    out_blob_desc->mut_shape() = Shape(dim_vec);
    out_blob_desc->set_data_type(blob_conf.data_type());
    out_blob_desc->set_has_data_id_field(Global<JobDesc>::Get()->SizeOfOneDataId() > 0);
    out_blob_desc->set_has_col_num_field(blob_conf.max_sequence_size() > 1);
    out_blob_desc->set_max_col_num(blob_conf.max_sequence_size());
    const auto& encode = blob_conf.encode_case();
    const auto* decoder_if = GetOFRecordDecoder(encode.encode_case(), blob_conf.data_type());
    out_blob_desc->set_has_dim1_valid_num_field(decoder_if->HasDim1ValidNumField(encode));
    out_blob_desc->set_has_dim2_valid_num_field(decoder_if->HasDim2ValidNumField(encode));
  }
}

LogicalBlobId DecodeOFRecordOp::obn2lbi(const std::string& output_bn) const {
  CHECK_STREQ(output_bn.substr(0, 4).c_str(), "out_");
  LogicalBlobId ret;
  ret.set_op_name(op_name());
  ret.set_blob_name(
      op_conf().decode_ofrecord_conf().blob(oneflow_cast<int32_t>(output_bn.substr(4))).name());
  return ret;
}

REGISTER_OP(OperatorConf::kDecodeOfrecordConf, DecodeOFRecordOp);

}  // namespace oneflow
