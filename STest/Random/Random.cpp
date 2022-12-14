// ======================================================================
// \title  Random.cpp
// \author bocchino
// \brief  Random number generation
//
// \copyright
// Copyright (C) 2017-2022 California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================

#include <cassert>
#include <ctime>

#include "Random.hpp"

extern "C" {
#include "bsd_random.h"
}

namespace STest {

  namespace Random {

    namespace SeedValue {

      U32 getFromTime() {
        struct timeval tv;
        (void) gettimeofday(&tv, nullptr);
        return tv.tv_usec;
      }

      bool getFromFile(
          const char *const fileName,
          U32& value
      ) {
        bool result = true;
        FILE *fp = fopen(fileName, "r");
        if (fp != nullptr) {
          result = (fscanf(fp, "%u", &value) == 1);
          (void) fclose(fp);
        }
        else {
          result = false;
        }
        return result;
      }

      void set(const U32 value) {
        bsd_srandom(value);
      }

      bool appendToFile(
          const char *const fileName,
          const U32 seedValue
      ) {
        bool result = true;
        FILE *fp = fopen(fileName, "a");
        if (fp != nullptr) {
          int status = fprintf(
              fp,
              "%u\n",
              seedValue
          );
          result = (status > 0);
          (void) fclose(fp);
        }
        else {
          result = false;
        }
        return result;
      }

    }

    void seed() {
      U32 seedValue = 0;
      const bool seedValueOK =
        SeedValue::getFromFile("seed", seedValue);
      if (seedValueOK) {
        printf("[STest::Random] Read seed %u from file\n", seedValue);
      }
      else {
        seedValue = SeedValue::getFromTime();
        printf("[STest::Random] Generated seed %u from system time\n", seedValue);
      }
      (void) SeedValue::appendToFile("seed-history", seedValue);
      SeedValue::set(seedValue);
    }

    U32 startLength(
        const U32 start,
        const U32 length
    ) {
      assert(length > 0);
      const F64 randFloat = inUnitInterval() * length;
      const U32 offset = static_cast<U32>(randFloat);
      return start + offset;
    }

    U32 lowerUpper(
        const U32 lower,
        const U32 upper
    ) {
      assert(lower <= upper);
      const F64 length = static_cast<F64>(upper) - lower + 1;
      const F64 randFloat = inUnitInterval() * length;
      const U32 offset = static_cast<U32>(randFloat);
      return lower + offset;
    }

    double inUnitInterval() {
      const U32 randInt = bsd_random();
      const F64 ratio = static_cast<F64>(randInt) / MAX_VALUE;
      return ratio;
    }

  }

}
