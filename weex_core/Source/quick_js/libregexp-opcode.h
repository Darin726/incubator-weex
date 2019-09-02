/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifdef DEF

DEF(invalid, 1) /* never used */
DEF(char, 3)
DEF(char32, 5)
DEF(dot, 1)
DEF(any, 1) /* same as dot but match any character including line terminator */
DEF(line_start, 1)
DEF(line_end, 1)
DEF(goto, 5)
DEF(split_goto_first, 5)
DEF(split_next_first, 5)
DEF(match, 1)
DEF(save_start, 2) /* save start position */
DEF(save_end, 2) /* save end position, must come after saved_start */
DEF(save_reset, 3) /* reset save positions */
DEF(loop, 5) /* decrement the top the stack and goto if != 0 */
DEF(push_i32, 5) /* push integer on the stack */
DEF(drop, 1)
DEF(word_boundary, 1)
DEF(not_word_boundary, 1)
DEF(back_reference, 2)
DEF(backward_back_reference, 2) /* must come after back_reference */
DEF(range, 3) /* variable length */
DEF(range32, 3) /* variable length */
DEF(lookahead, 5)
DEF(negative_lookahead, 5)
DEF(push_char_pos, 1) /* push the character position on the stack */
DEF(bne_char_pos, 5) /* pop one stack element and jump if equal to the character
 position */
DEF(prev, 1) /* go to the previous char */
DEF(simple_greedy_quant, 17)

#endif /* DEF */
