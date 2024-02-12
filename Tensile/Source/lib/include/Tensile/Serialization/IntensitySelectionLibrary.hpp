
/*******************************************************************************
 *
 * MIT License
 *
 * Copyright (C) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#pragma once

#include <Tensile/IntensitySelectionLibrary.hpp>
#include <Tensile/MasterSolutionLibrary.hpp>

namespace Tensile
{
    namespace Serialization
    {

        template <typename MyProblem, typename MySolution, typename IO>
        struct MappingTraits<IntensitySelectionLibrary<MyProblem, MySolution>, IO>
        {
            using Library = IntensitySelectionLibrary<MyProblem, MySolution>;

            using iot = IOTraits<IO>;

            static void mapping(IO& io, Library& lib)
            {
                auto ctx = static_cast<LibraryIOContext<MySolution>*>(iot::getContext(io));
                if(ctx == nullptr)
                {
                    iot::setError(io,
                                  "IntensitySelectionLibrary requires that context be "
                                  "set to a SolutionMap.");
                }

                std::vector<int> mappingIndices;
                if(iot::outputting(io))
                {
                    mappingIndices.reserve(lib.solutions.size());

                    for(auto const& pair : lib.solutions)
                        mappingIndices.push_back(pair.first);

                    iot::mapRequired(io, "indices", mappingIndices);
                }
                else
                {
                    iot::mapRequired(io, "indices", mappingIndices);
                    if(mappingIndices.empty())
                        iot::setError(io,
                                      "IntensitySelectionLibrary requires non empty "
                                      "mapping index set.");

                    for(int index : mappingIndices)
                    {
                        auto slnIter = ctx->solutions->find(index);
                        if(slnIter == ctx->solutions->end())
                        {
                            iot::setError(io, concatenate("Invalid solution index: ", index));
                        }
                        else
                        {
                            auto solution = slnIter->second;
                            lib.solutions.insert(std::make_pair(index, solution));
                        }
                    }
                }
            }
        };
    } // namespace Serialization
} // namespace Tensile
