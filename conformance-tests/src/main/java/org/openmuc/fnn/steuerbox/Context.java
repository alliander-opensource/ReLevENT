/*
 * Copyright 2023 Fraunhofer ISE
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

package org.openmuc.fnn.steuerbox;

import javax.xml.parsers.DocumentBuilderFactory;

/**
 * Holds shared objects
 */
public class Context {

    private static final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();

    public static DocumentBuilderFactory getDocumentBuilderFactory() {
        return factory;
    }
}
