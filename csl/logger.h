#pragma once

#ifndef CSL_LOGGER_H
#define CSL_LOGGER_H

#include "util/ioutil.h"
#include "util/errors.h"

#include <ostream>

void output_error(std::ostream&, const CSLError&, const StrReader*);

void output_context(std::ostream&, const StrReader*);

#endif