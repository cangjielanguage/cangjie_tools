#ifndef CJPROF_HTTP_HANDLERS_H
#define CJPROF_HTTP_HANDLERS_H

#include <string>
#include <memory>
#include "Analyzer/HttpContext.h"

namespace cjprof {

class HttpHandlers {
public:
    static std::string handleSnapshot(const HttpContext& ctx);
    static std::string handleDominanceTree(const HttpContext& ctx);
    static std::string handleDominanceTreeByType(const HttpContext& ctx);
    static std::string handleDominanceChildren(const HttpContext& ctx, uint64_t parentId);
    static std::string handleDominanceChildrenByType(const HttpContext& ctx, const std::string& nodeId);
    static std::string handleDominanceClusterExpand(const HttpContext& ctx, const std::vector<uint64_t>& instanceIds);
    static std::string handleDominanceTop10(const HttpContext& ctx);
    static std::string handleFragmentOverview(const HttpContext& ctx);
    static std::string handleFragmentLayout(const HttpContext& ctx);
    static std::string handleFragmentSummary(const HttpContext& ctx);
};

} // namespace cjprof

#endif // CJPROF_HTTP_HANDLERS_H
