// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#ifndef CREATE_OUTPUT_MASK_USING_SETTER_GETTER
#  define CREATE_OUTPUT_MASK_USING_SETTER_GETTER

namespace ChimeraTK {

  /* To check the position and the length of the bit pattern we use reconstruct
   * the output mask. To do so we need the larged allowed bit sequence to use with
   * the setter function. To get this we initialise the word with all bits on and
   * call the getter function. Afterwords we reset all bits to 0 and use the
   * setter function with this larges possible input word.
   */
  template<class T>
  unsigned int createOutputMaskUsingSetterGetter(
      void (T::*setterFunction)(unsigned int), unsigned int (T::*getterFunction)() const) {
    T t_max;
    t_max.setDataWord(0xFFFFFFFF); // this basically turns the data word invalid
                                   // because the register bits are all 1
    unsigned int largestPossibleSubWord = (t_max.*getterFunction)();

    // create a new, valid data word (register bits set correctly)
    T t;
    (t.*setterFunction)(largestPossibleSubWord);
    return t.getDataWord();
  }

} // namespace ChimeraTK

#endif // CREATE_OUTPUT_MASK_USING_SETTER_GETTER
