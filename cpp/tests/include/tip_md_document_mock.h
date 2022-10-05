#ifndef TIP_MD_DOCUMENT_MOCK_H_
#define TIP_MD_DOCUMENT_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tip_md_document.h"

class MockTIPMDDocument : public TIPMDDocument
{
   public:
    MockTIPMDDocument() : TIPMDDocument() {}
    MOCK_METHOD1(ReadDocument, bool(const std::string& doc));
    MOCK_METHOD0(CreateDocument, bool());
    MOCK_METHOD0(GetMetadataString, std::string());
    MOCK_METHOD0(GetConfigCategory, std::shared_ptr<MDCategoryMap>());
    MOCK_METHOD0(GetRuntimeCategory, std::shared_ptr<MDCategoryMap>());
};

#endif  // TIP_MD_DOCUMENT_MOCK_H_
