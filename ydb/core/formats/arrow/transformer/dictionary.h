#pragma once
#include "abstract.h"

namespace NKikimr::NArrow::NTransformation {

class TDictionaryPackTransformer: public ITransformer {
protected:
    virtual std::shared_ptr<arrow::RecordBatch> DoTransform(const std::shared_ptr<arrow::RecordBatch>& batch) const override;
public:
};

class TDictionaryUnpackTransformer: public ITransformer {
protected:
    virtual std::shared_ptr<arrow::RecordBatch> DoTransform(const std::shared_ptr<arrow::RecordBatch>& batch) const override;
public:
};

}
