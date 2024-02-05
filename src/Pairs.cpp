#include "Pairs.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <assert.h>

Pairs::Pairs(/* args */) : size(0)
{
}

Pairs::Pairs(const std::string &filename, const std::size_t _size) :  size(_size) {
  read(filename);
}

Pairs::~Pairs()
{
}

void Pairs::clearValues() {
  for (auto v : values) {
    v.clear();
  }
  values.clear();
}

void Pairs::set_size(const std::size_t _size) {
  size = _size;
}

void Pairs::read(const std::string &filename) {
  assert(size > 0);
  
  std::string tmpStr, s;
	std::istringstream iss;
	std::ifstream input;

  clearValues();
  std::vector<unsigned int> valuesRow;
  for (std::size_t i = 0; i < size; ++i) {
    values.push_back(valuesRow);
  }

	// Open the file
	input.open(filename.c_str());

	// Check if file opened
	if (!input) {
    fprintf(stderr, "ERROR - Could not open file (%s)\n", filename.c_str());		
		exit(1);
	}

  // Loop through file, reading each line
  while (std::getline(input, tmpStr)) {
    // Count number of comas
    std::size_t numPairs = std::count(tmpStr.begin(), tmpStr.end(), ',') + 1;
    // Clear and update istringstream
    iss.clear();
    iss.str(tmpStr);

    while (std::getline(iss, s, ',')) {
      values[size-numPairs].push_back(std::stoi(s));
    }
  }

  input.close();
}

void Pairs::print() {
  for (auto vec : values) {
    for (auto v : vec) {
      fprintf(stderr, "%u ", v);
    }
    fprintf(stderr, "\n");
  }
}

std::vector<std::size_t> Pairs::getPairsGteThresh(const std::size_t idx, const unsigned int threshold) const {
  assert(idx < values.size());
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] >= threshold) {
      pairs.push_back(i);
    }
  }

  for (std::size_t j = 0; j < values[idx].size(); ++j) {
    if (values[idx][j] >= threshold) {
      pairs.push_back(idx + 1 + j);
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold) const {
  assert(idx < values.size());
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] < threshold) {
      pairs.push_back(i);
    }
  }

  for (std::size_t j = 0; j < values[idx].size(); ++j) {
    if (values[idx][j] < threshold) {
      pairs.push_back(idx + 1 + j);
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {
  assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] < threshold) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] && values[idx][j] < threshold) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::vector<std::size_t> Pairs::getPairsLtThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
    assert(idx < values.size());
  
  std::vector<std::size_t> pairs;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] < threshold) {
      pairs.push_back(i);
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx+1+j] && values[idx][j] < threshold) {
        pairs.push_back(idx + 1 + j);
      }
    }
  }
  return pairs;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold) const {
  assert(idx < values.size());
  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  return count;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<bool> valid) const {  
  assert(idx < valid.size());

  if (!valid[idx]) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] && values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  
  return count;
}

std::size_t Pairs::getNumPairsGteThresh(const std::size_t idx, const unsigned int threshold, const std::vector<int> valid) const {
  assert(idx < valid.size());

  if (!valid[idx]) {
    return 0;
  }

  std::size_t count = 0;

  for (std::size_t i = 0; i < idx; ++i) {
    if (valid[i] && values[i][idx-i-1] >= threshold) {
      ++count;
    }
  }

  if (idx < values.size()) {
    for (std::size_t j = 0; j < values[idx].size(); ++j) {
      if (valid[idx + 1 + j] && values[idx][j] >= threshold) {
        ++count;
      }
    }
  }
  
  return count;
}


void Pairs::recalculateValues(const bool rowCol,
                              const std::vector<std::size_t> &freeDim1,
                              const std::vector<std::size_t> &freeDim2,
                              const std::vector<bool> &validDim2,
                              const std::size_t numDim2ForcedToOne,
                              const BinContainer &data) {
  assert(size == freeDim1.size() - 1);
  assert(freeDim2.size() == validDim2.size());

  // fprintf(stderr, "Size: %lu - New Size: %lu\n", size, freeDim1.size() - 1);
  // fprintf(stderr, "Dim2 size: %lu - Dim2 valid size: %lu\n", freeDim2.size(), validDim2.size());


  if (rowCol) {
    for (std::size_t localI1 = 0; localI1 < freeDim1.size()-1; ++localI1) {
      const std::size_t i1 = freeDim1[localI1];

      // fprintf(stderr, "pairRow row size: %lu\n", values[localI1].size());
      // fprintf(stderr, "pairs to check in row: %lu\n", freeDim1.size()-1 - localI1);

      for (std::size_t localI2 = 0; localI2 < values[localI1].size(); ++localI2) {
        const std::size_t i2 = freeDim1[localI1 + 1 + localI2];

        values[localI1][localI2] = numDim2ForcedToOne;

        for (std::size_t localJ = 0; localJ < freeDim2.size(); ++localJ) {
          if (!validDim2[localJ]) continue;

          std::size_t j = freeDim2[localJ];

          if (!data.is_data_na(i1, j) && !data.is_data_na(i2, j)) {
            ++values[localI1][localI2];
          }
        }

        // fprintf(stderr, "Count (%lu, %lu): %u\n", i1, i2, values[localI1][localI2]);
      }
    }
  } else {
    fprintf(stderr, "Not implmented for cols\n");
    exit(1);
  }
  
}