//
// Created by Dániel Molnár on 2019-08-20.
//

#pragma once
#ifndef CORE_IGRAPHNODE_HPP
#define CORE_IGRAPHNODE_HPP

// ----- std -----
#include <vector>
#include <memory>

// ----- libraries -----

// ----- in-project dependencies -----

// ----- forward-decl -----

namespace Core {
class IGraphNode {
  public:
    virtual std::vector<std::shared_ptr<IGraphNode>> parents() const = 0;
    virtual std::vector<std::shared_ptr<IGraphNode>> children() const = 0;

};
} // namespace Core

#endif // CORE_IGRAPHNODE_HPP
