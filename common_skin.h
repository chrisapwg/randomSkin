#pragma once
#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>

#include <maya/MObject.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <set>
#include <unordered_map>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnTransform.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>

class common_Function : public MPxCommand {
public:
    int getDecimal(double prune) {
        if (prune > 0.1) {
            return 2;
        }
        else if (prune > 0.01) {
            return 3;
        }
        else if (prune > 0.001) {
            return 4;
        }
        return 5;
    }

    void printMapIntDouble(const std::map<int, std::vector<double>>& _map_allWithNeighbor) {
        for (auto it = _map_allWithNeighbor.begin(); it != _map_allWithNeighbor.end(); ++it) {
            int _index = it->first;
            const std::vector<double>& _w = it->second;

            MString output;
            output += _index;
            output += ": [";

            for (size_t i = 0; i < _w.size(); ++i) {
                output += _w[i];
                if (i < _w.size() - 1) output += ", ";
            }

            output += "]";

            MGlobal::displayInfo(output);
        }
    }

    void printMIntArray(const MString& arrayName, const MIntArray& arrayData) {
        MString outputString = "-List '" + arrayName + "' = [";
        for (unsigned int i = 0; i < arrayData.length(); ++i) {
            outputString += MString() + arrayData[i];
            if (i < arrayData.length() - 1) {
                outputString += ", ";
            }
        }
        outputString += "]";
        MGlobal::displayInfo(outputString);
    }

    void printMDoubleArray(const MString& arrayName, const MDoubleArray& arrayData) {
        MString outputString = "-List '" + arrayName + "' = [";
        float total = 0;
        for (unsigned int i = 0; i < arrayData.length(); ++i) {
            outputString += MString() + arrayData[i];
            total += arrayData[i];
            if (i < arrayData.length() - 1) {
                outputString += ", ";
            }
        }
        outputString += "]";
        MGlobal::displayInfo(outputString);
        MGlobal::displayInfo(MString("The total sum = ") + total);
        MGlobal::displayInfo(MString("The length = ") + arrayData.length());
    }

    void printMFloatArray(const MString& arrayName, const MFloatArray& arrayData) {
        MString outputString = "-List '" + arrayName + "' = [";
        for (unsigned int i = 0; i < arrayData.length(); ++i) {
            outputString += MString() + arrayData[i];
            if (i < arrayData.length() - 1) {
                outputString += ", ";
            }
        }
        outputString += "]";
        MGlobal::displayInfo(outputString);
    }

    void sortMIntArray(MIntArray& array) {
        std::vector<int> vec(array.length());
        for (unsigned int i = 0; i < array.length(); ++i)
            vec[i] = array[i];

        std::sort(vec.begin(), vec.end());

        array.clear();
        for (unsigned int i = 0; i < vec.size(); ++i)
            array.append(vec[i]);
    }

    void printMStringArray(const MString& arrayName, const MStringArray& arrayData) {
        MString outputString = "-List '" + arrayName + "' = [";
        for (unsigned int i = 0; i < arrayData.length(); ++i) {
            outputString += arrayData[i];
            if (i < arrayData.length() - 1) {
                outputString += ", ";
            }
        }
        outputString += "]";
        MGlobal::displayInfo(outputString);
    }

    void printVectorInt(const std::vector<int>& indices) {
        MString msg = "Vector Int Print : ";

        for (size_t i = 0; i < indices.size(); ++i) {
            msg += indices[i];
            if (i < indices.size() - 1) {
                msg += ", ";
            }
        }

        MGlobal::displayInfo(msg);
    }
    /*
    void normalizeWeights(MDoubleArray& weights, std::vector<bool> lockedJoints, double epsilon = 0.0001, int decimals = 5, double prune = 0.001, unsigned int maxInfluence = 4) {
        MDoubleArray originalWeights = weights;  // Keep a copy for reference
        MDoubleArray prunedWeights(weights.length(), 0.0);

        // ----- Step 1: Prune small weights -----
        for (unsigned int i = 0; i < weights.length(); ++i) {
            double val = weights[i];
            if (!lockedJoints[i]) {
                if ((val > prune && val <= 1.0) || fabs(val - 1.0) < epsilon) {
                    prunedWeights[i] = val;
                }
            }
            else {
                prunedWeights[i] = val;
            }
        }

        // ----- Step 2: Limit max influences -----
        if (maxInfluence != 0) {
            std::vector<double> weightVector(prunedWeights.begin(), prunedWeights.end());

            while (std::count_if(weightVector.begin(), weightVector.end(), [&](double w) { return w > 0.0; }) > maxInfluence) {
                double minWeight = std::numeric_limits<double>::max();
                unsigned int minIndex = -1;

                for (unsigned int i = 0; i < weightVector.size(); ++i) {
                    if (weightVector[i] > 0.0 && weightVector[i] < minWeight) {
                        if (!lockedJoints[i]) {
                            minWeight = weightVector[i];
                            minIndex = i;
                    }
                }

                if (minIndex != -1) {
                    prunedWeights[minIndex] = 0.0;
                    weightVector[minIndex] = 0.0;
                }
            }
        }

        // Copy pruned values back
        weights = prunedWeights;

        // ----- Step 3: Normalize -----
        double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
        if (fabs(sum - 1.0) > epsilon && sum > 0.0) {
            for (unsigned int i = 0; i < weights.length(); ++i) {
                if (!lockedJoints[i]) {
                    weights[i] /= sum;
                }
            }
        }

        // ----- Step 4: Round decimals -----
        double factor = std::pow(10.0, decimals);
        for (unsigned int i = 0; i < weights.length(); ++i) {
            weights[i] = std::round(weights[i] * factor) / factor;
        }

        // ----- Step 5: Fix floating point error -----
        sum = std::accumulate(weights.begin(), weights.end(), 0.0);
        double error = sum - 1.0;
        if (fabs(error) > epsilon) {
            unsigned int maxIndex = 0;
            for (unsigned int i = 1; i < weights.length(); ++i) {
                if (weights[i] > weights[maxIndex]) {
                    maxIndex = i;
                }
                if (lockedJoints[maxIndex]) {
                    maxIndex = i;
                }
            }
            weights[maxIndex] -= error;
        }

        // ----- Step 6: Clamp out-of-bound values -----
        for (unsigned int i = 0; i < weights.length(); ++i) {
            if (!lockedJoints[i]) {
                if (weights[i] < epsilon) weights[i] = 0.0;
                if (weights[i] < 0.0) weights[i] = 0.0;
                if (weights[i] > 1.0) weights[i] = 1.0;
            }
        }
    }
    */
    void normalizeWeights(MDoubleArray& weights, std::vector<bool> lockedJoints, double epsilon = 0.0001, int decimals = 5, double prune = 0.001, unsigned int maxInfluence = 4) {
        MDoubleArray originalWeights = weights;
        MDoubleArray prunedWeights(weights.length(), 0.0);

        // ----- Step 1: Prune small weights -----
        for (unsigned int i = 0; i < weights.length(); ++i) {
            if (lockedJoints[i] || weights[i] > prune || fabs(weights[i] - 1.0) < epsilon) {
                prunedWeights[i] = weights[i];
            }
        }

        // ----- Step 2: Limit max influences -----
        if (maxInfluence != 0) {
            std::vector<double> weightVector(prunedWeights.begin(), prunedWeights.end());

            // Count locked weights > 0
            unsigned int lockedCount = 0;
            for (unsigned int i = 0; i < weightVector.size(); ++i) {
                if (lockedJoints[i] && weightVector[i] > 0.0) {
                    ++lockedCount;
                }
            }

            // If locked weights already exceed max, skip pruning
            if (lockedCount < maxInfluence) {
                unsigned int allowedUnlocked = maxInfluence - lockedCount;

                auto countUnlocked = [&]() {
                    unsigned int count = 0;
                    for (unsigned int i = 0; i < weightVector.size(); ++i) {
                        if (!lockedJoints[i] && weightVector[i] > 0.0)
                            ++count;
                    }
                    return count;
                    };

                // Prune smallest unlocked weights until within limit
                while (countUnlocked() > allowedUnlocked) {
                    double minWeight = std::numeric_limits<double>::max();
                    int minIndex = -1;

                    for (unsigned int i = 0; i < weightVector.size(); ++i) {
                        if (!lockedJoints[i] && weightVector[i] > 0.0 && weightVector[i] < minWeight) {
                            minWeight = weightVector[i];
                            minIndex = i;
                        }
                    }

                    if (minIndex == -1) {
                        // Nothing left to remove
                        break;
                    }

                    weightVector[minIndex] = 0.0;
                    prunedWeights[minIndex] = 0.0;
                }
            }
            // else: skip pruning because locked already consume all influence slots
        }

        // Copy pruned values back
        weights = prunedWeights;
        double sum = std::accumulate(weights.begin(), weights.end(), 0.0);

        // ----- Step 3: Normalize -----
        if (fabs(sum - 1.0) > epsilon && sum > 0.0) {
            for (unsigned int i = 0; i < weights.length(); ++i) {
                if (!lockedJoints[i]) {
                    weights[i] /= sum;
                }
            }
        }

        // ----- Step 4: Round decimals -----
        double factor = std::pow(10.0, decimals);
        for (unsigned int i = 0; i < weights.length(); ++i) {
            weights[i] = std::round(weights[i] * factor) / factor;
        }

        // ----- Step 5: Fix floating-point error -----
        sum = std::accumulate(weights.begin(), weights.end(), 0.0);
        double error = sum - 1.0;
        if (fabs(error) > epsilon) {
            int maxIndex = -1;
            double maxValue = -1.0;

            for (unsigned int i = 0; i < weights.length(); ++i) {
                if (!lockedJoints[i] && weights[i] > maxValue) {
                    maxValue = weights[i];
                    maxIndex = i;
                }
            }

            if (maxIndex >= 0) {
                weights[maxIndex] -= error;
            }
        }

        // ----- Step 6: Clamp values -----
        for (unsigned int i = 0; i < weights.length(); ++i) {
            if (!lockedJoints[i]) {
                if (weights[i] < epsilon || weights[i] < 0.0) weights[i] = 0.0;
                if (weights[i] > 1.0) weights[i] = 1.0;
            }
        }
    }

};
