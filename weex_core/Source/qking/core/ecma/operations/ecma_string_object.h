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
#ifndef ECMA_STRING_OBJECT_H
#define ECMA_STRING_OBJECT_H

#include "ecma_globals.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmastringobject ECMA String object related routines
 * @{
 */

ecma_value_t ecma_op_create_string_object(const ecma_value_t *arguments_list_p,
                                          ecma_length_t arguments_list_len);

void ecma_op_string_list_lazy_property_names(
    ecma_object_t *obj_p, bool separate_enumerable,
    ecma_collection_header_t *main_collection_p,
    ecma_collection_header_t *non_enum_collection_p);

/**
 * @}
 * @}
 */

#endif /* !ECMA_STRING_OBJECT_H */
