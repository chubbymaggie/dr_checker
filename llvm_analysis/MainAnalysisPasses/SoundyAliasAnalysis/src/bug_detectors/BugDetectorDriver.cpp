//
// Created by machiry on 12/28/16.
//

#include "bug_detectors/BugDetectorDriver.h"
#include "bug_detectors/TaintedPointerDereference.h"
#include "bug_detectors/KernelUninitMemoryLeakDetector.h"
#include "bug_detectors/TaintedSizeDetector.h"
#include "bug_detectors/InvalidCastDetector.h"
#include "bug_detectors/IntegerOverflowDetector.h"
#include "bug_detectors/ImproperTaintedDataUseDetector.h"
#include "bug_detectors/TaintedLoopBoundDetector.h"
#include "bug_detectors/GlobalVariableRaceDetector.h"

using namespace llvm;

namespace DRCHECKER {

    void BugDetectorDriver::addPreAnalysisBugDetectors(GlobalState &targetState,
                                                       Function *toAnalyze,
                                                       std::vector<Instruction *> *srcCallSites,
                                                       std::vector<VisitorCallback *> *allCallbacks,
                                                       FunctionChecker *targetChecker) {

        VisitorCallback *invalidCastDetector = new InvalidCastDetector(targetState,
                                                                       toAnalyze,
                                                                       srcCallSites, targetChecker);
        allCallbacks->push_back(invalidCastDetector);

        VisitorCallback *integerOverflowDetector = new IntegerOverflowDetector(targetState,
                                                                           toAnalyze,
                                                                           srcCallSites, targetChecker);
        allCallbacks->push_back(integerOverflowDetector);

    }

    void BugDetectorDriver::addPostAnalysisBugDetectors(GlobalState &targetState,
                                                        Function *toAnalyze,
                                                        std::vector<Instruction *> *srcCallSites,
                                                        std::vector<VisitorCallback *> *allCallbacks,
                                                        FunctionChecker *targetChecker) {

        VisitorCallback *currTaintDetector = new TaintedPointerDereference(targetState,
                                                                           toAnalyze,
                                                                           srcCallSites, targetChecker);
        allCallbacks->push_back(currTaintDetector);

        VisitorCallback *currLeakDetector = new KernelUninitMemoryLeakDetector(targetState,
                                                                               toAnalyze,
                                                                               srcCallSites, targetChecker);
        allCallbacks->push_back(currLeakDetector);

        VisitorCallback *currTaintSizeDetector = new TaintedSizeDetector(targetState,
                                                                         toAnalyze,
                                                                         srcCallSites, targetChecker);
        allCallbacks->push_back(currTaintSizeDetector);

        VisitorCallback *currImproperDataUseDetector = new ImproperTaintedDataUseDetector(targetState,
                                                                                          toAnalyze,
                                                                                          srcCallSites, targetChecker);
        allCallbacks->push_back(currImproperDataUseDetector);

        VisitorCallback *taintedCondDetector = new TaintedLoopBoundDetector(targetState,
                                                                            toAnalyze,
                                                                            srcCallSites, targetChecker);
        allCallbacks->push_back(taintedCondDetector);

        VisitorCallback *globalVarRaceDetector = new GlobalVariableRaceDetector(targetState,
                                                                                toAnalyze,
                                                                                srcCallSites, targetChecker);
        allCallbacks->push_back(globalVarRaceDetector);

    }

    void BugDetectorDriver::printAllWarnings(GlobalState &targetState, llvm::raw_ostream& O) {
        O << "{\"num_contexts\":";
        if(targetState.allVulnWarnings.size() == 0) {
            O << "0";
            //O << "No Warnings. Everything looks good\n";
        } else {
            O << targetState.allVulnWarnings.size() << ",\n";
            bool addout_comma = false;
            O << "\"all_contexts\":[\n";
            for (auto warn_iter = targetState.allVulnWarnings.begin(); warn_iter != targetState.allVulnWarnings.end();
                 warn_iter++) {
                bool addin_comma = false;
                if(addout_comma) {
                    O << ",\n";
                }
                O << "{";
                AnalysisContext *targetContext = warn_iter->first;
                std::set<VulnerabilityWarning *> *allWarnings = warn_iter->second;
                O << "\"num_warnings\":" << allWarnings->size() << ",\n";
                // O << "At Calling Context:";
                targetContext->printContext(O);
                O << ",";

                //O << "Found:" << allWarnings->size() << " warnings.\n";
                long currWarningNo = 1;
                O << "\"warnings\":[\n";
                for (VulnerabilityWarning *currWarning:*(allWarnings)) {
                    if(addin_comma) {
                        O << ",\n";
                    }
                    O << "{";
                    O << "\"warn_no\":" << currWarningNo << ",";
                    currWarning->printWarning(O);
                    currWarningNo++;
                    addin_comma = true;
                    O << "}";
                }
                O << "\n]";
                addout_comma = true;
                O << "}";
            }
            O << "]\n";
        }
        O << "\n}";
    }
}
