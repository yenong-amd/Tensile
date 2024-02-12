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

#include <set>
#include <vector>

#include <Tensile/Debug.hpp>
#include <Tensile/Properties.hpp>
#include <Tensile/Utils.hpp>

#include <Tensile/PropertyMatching.hpp>

namespace Tensile
{
    /**
 * \ingroup SolutionLibrary
 *
 * Selects Stream K kernel based on problem arithmetic intensity
 */
    template <typename MyProblem, typename MySolution = typename MyProblem::Solution>
    struct IntensitySelectionLibrary : public SolutionLibrary<MyProblem, MySolution>
    {
        std::map<int, std::shared_ptr<MySolution>> solutions;

        static std::string Type()
        {
            return "IntensitySelection";
        }
        virtual std::string type() const override
        {
            return Type();
        }
        virtual std::string description() const override
        {
            std::string rv = this->type();

            return rv;
        }

        virtual std::shared_ptr<MySolution> findBestSolution(MyProblem const& problem,
                                                             Hardware const&  hardware,
                                                             double*          fitness
                                                             = nullptr) const override
        {
            const bool debug = Debug::Instance().printPropertyEvaluation();

            std::vector<size_t> key;
            size_t              M = problem.freeSizeA(0);
            key.push_back(M);
            size_t N = problem.freeSizeB(0);
            key.push_back(N);
            size_t NumBatches = problem.batchSize(0);
            key.push_back(NumBatches);
            size_t K = problem.boundSize(0);
            key.push_back(K);

            double                      bestDiff         = std::numeric_limits<double>::max();
            double                      problemIntensity = problem.arithmeticIntensity();
            std::shared_ptr<MySolution> bestSolution;

            std::cout << "IntensitySelection M " << M << ", N " << N << ", K " << K << std::endl;
            std::cout << "Arithmetic intensity " << problemIntensity << std::endl;
            for(auto const& row : solutions)
            {
                auto myDiff = row.second->streamKSolution(problemIntensity, K);

                if(debug)
                {
                    std::cout << row.second->description() << ": " << myDiff << " index "
                              << row.second->index << std::endl;
                }

                if(myDiff < 0.0)
                {
                    continue;
                }
                else if(myDiff < bestDiff)
                {
                    if((*row.second->problemPredicate)(problem)
                       && (*row.second->hardwarePredicate)(hardware))
                    {
                        bestDiff     = myDiff;
                        bestSolution = row.second;

                        if(debug)
                            std::cout << " <-- Best so far";
                    }
                    else if(debug)
                    {
                        std::cout << " <-- Best, but predicate failure";
                    }

                    if(debug)
                    {
                        row.second->problemPredicate->debugEval(problem, std::cout);
                        std::cout << std::endl;
                        row.second->hardwarePredicate->debugEval(hardware, std::cout);
                        std::cout << std::endl;
                    }
                }
            }

            return bestSolution;
        }

        virtual SolutionSet<MySolution> findAllSolutions(MyProblem const& problem,
                                                         Hardware const&  hardware) const override
        {
            bool debug = Debug::Instance().printPropertyEvaluation();

            SolutionSet<MySolution> rv;

            for(auto const& row : solutions)
            {
                if(debug)
                {
                    std::cout << row.second->description() << ": ";
                }

                if((*row.second->problemPredicate)(problem)
                   && (*row.second->hardwarePredicate)(hardware))
                {
                    rv.insert(row.second);

                    if(debug)
                        std::cout << " Works";
                }
                else if(debug)
                {
                    if(debug)
                        std::cout << " Predicate failed";
                }

                if(debug)
                {
                    row.second->problemPredicate->debugEval(problem, std::cout);
                    std::cout << std::endl;
                    row.second->hardwarePredicate->debugEval(hardware, std::cout);
                    std::cout << std::endl;
                }
            }

            return rv;
        }

        virtual SolutionSet<MySolution>
            findAllSolutionsMatchingType(MyProblem const& problem,
                                         Hardware const&  hardware) const override
        {
            SolutionSet<MySolution> rv;

            for(auto const& row : solutions)
            {
                if(row.second->matchesProblemType(problem, hardware))
                {
                    rv.insert(row.second);
                }
            }

            return rv;
        }
    };
} // namespace Tensile
