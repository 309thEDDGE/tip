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
};

#endif  // TIP_MD_DOCUMENT_MOCK_H_
