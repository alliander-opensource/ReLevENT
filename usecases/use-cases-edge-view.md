# Possible use-cases (from edge view)

The following use-cases are described based on the (business) value that is (or will be) available in the edge, although we make use of many examples where the DSO is involved. In this case "edge" is defined as grid-connected-customers with their schedule-control-software. The purpose of these use-cases is to get an impression on the abilities of this software and some basic considerations on the implementation. They serve as a starter to make more serious architectures.

Other use-cases sections at this repository will describe what to take into consideration from central perspective, like systems of the transmission system operator (TSO), [distribution system operator (DSO)](./use-cases-DSOcentral-view.MD), charging point operator (CPO), end-customers, etc.

Note: if use-cases describe "schedules sent to edge-software" (=push) it could also mean: "the edge-software retrieves schedules from a central system" (=pull) and the other way around.

**Index (in order of increasing value):**

1.  a direct command from 1 remote actor
2.  one type of scheduling command from 1 remote actor
3.  more scheduling commands from 1 remote actor
4.  more scheduling commands from 1 remote actor and local generated commands
5.  more scheduling commands from 2 remote actors using the same protocols
6.  more scheduling commands from more remote actors using all their own protocols
    

## Use-case 1: a direct command from 1 remote actor

| Description | One actor wants to control the edge software with a direct command |
| --- | --- |
| **Example:** | a Distribution System Operator (DSO) wants to curtail a wind park with its SCADA system to avoid overload of the grid which wasn't detected in advance. |
| **Expected value of this project:** | The edge automation software is prepared for more difficult use-cases as this is the bare minimum functionality that can be expected. See follow up use-cases. |
| **An implementation example 1:** | **Classical integration**: IEC 60870-5-104 communication from SCADA to edge-software. Fledge Power is configured to connect the 104 signals to the Scheduling software as a direct command. This configuration is performed by a configuration file in SCL according to the IEC 61850 standard. |
| **An implementation example 2:** | **Expected integration**: Same as above, but the central system uses IEC 61850 communication (e.g. MMS) and might be a SCADA, DERMS or more customer facing system  system being perfectly able to make integrations between customer interaction at a broad scale. |

## Use-case 2: one type of scheduling command from 1 remote actor

| Description | One actor wants to provide the edge software a control schedule so that the edge software already knows when and what to control in advance. |
| --- | --- |
| **Example:** | a Distribution System Operator (DSO) planned to perform maintenance on the grid at June 23rd from 9:00 to 11:00 which limits the available transport capacity. The edge software now already knows that during this time period only 30kW feed in is allowed instead of 45kW. |
| **Expected value of this project:** | (a) Resilience in control in case of (tele)communication break-down during maintenance work, (b) transparency in advance, (c) ability to optimize by other actors based on this information (e.g. having a empty battery at this moment to charge during 9:00 and 11:00). |
| **An implementation example 1:** | The edge-software has a schedule configured which is fed with the information from a central/cloud TSO system on the maintenance leading to a limit of 30kW infeed as a maximum. |
| **An implementation example 2:** | Fieldworkers from the TSO finishes their work early at 10:15 and notify this in their app. The edge-software can get the newly updated schedule and knows that the restricted limit (was) only from 9:00 to 10:15. |

## Use-case 3: more scheduling commands from 1 remote actor

| **Description:** | One actor want to provide the edge software with more control schedules so that in the edge it's clear what are the reasons and the priorities between them so it's understandable how the local control behaves. |
| --- | --- |
| **Example 1:** | A safe schedule is always available if there are no updates on the "normal" schedule. |
| **Example 2:** | A DSO-maintenance & outage schedule is always available and only limiting when this is planned at the DSO (see use-case above). |
| **Example 3:** | The DSO has two operational constraints; 1 locally in the grid with 20 other connected grid users and one constraint delegated from his transmission system operator (TSO). The edge-software could host both constraints separately in a different schedule. |
| **Expected value of this project:** | (a) increased transparency in actions and reasons of the actions of the actor (DSO in this example above), (b) able to optimize based on the limiting reason (in example 3 it would make sense to look with the 20 other users how to solve a limit and have an other optimization at TSO level), (c) For example 3 there is the option that the TSO limit can be managed more easiliy in central systems than managing this as an individual limit. |
| **An implementation example:** | Using the core scheduling functionality of the edge software (this project) to run more schedules in parallel and see implementation examples of use-cases above. |

## Use-case 4: more scheduling commands from 1 remote actor and local generated commands

| **Description:** | One actor want to provide the edge software with more control schedules so that in the edge it's clear what are the reasons AND the conditions that local controlling takes place |
| --- | --- |
| **Example 1:** | A "no telecom schedule" is always available and limits after a certain time that the condition is fulfilled that not telecom connectivity is available. |
| **Example 2:** | A "power-up schedule" is always available that is prioritized when the edge-device was without power up to x minutes ago. |
| **Example 3:** | Some schedules with constraints are available at the edge software that are activated in case the voltage level is higher than 239 Volts in order to avoid voltage problems higher in the grid. |
| **Expected value of this project:** | See earlier use-cases plus this software provides a generic way of controlling in edge devices with flexibility to prioritize remote or local schedules based on preconfigured priorities. |
| **An implementation example:** | Using the core scheduling functionality of the edge software (this project) to run more schedules in parallel and see implementation examples of use-cases above. |

## Use-case 5: more scheduling commands from 2 remote actors using the same protocols

| **Description:** | Two actors want to provide the edge software with more control schedules so that in the edge it's clear what are the reasons and the priorities between them so it's understandable how the local control behaves. |
| --- | --- |
| **Example:** | See Use-case 3 as this is another possibility how to realize that DSO manages very local constraints, TSO manages larger groups of constraints. The customer's automation can connect both to DSO and TSO to be controled / get highest possible safe capacity limit. |
| **Expected value of this project:** | See earlier use-cases . In addition: this use-case shows that it's even possible to have more actors on the same software-stack as long as the policy "who first?" and "in what situation first?" is preprogrammed. |
| **An implementation example 1:** | Using the core scheduling functionality of the edge software (this project) to run more schedules in parallel from more actors. |
| **An implementation example 2:** | The configuration [(**SLC** configuration)](../models/der_scheduler.cid) of the schedules let's allow (or disallow) an actor to put have limits in the edge software and... |
| **An implementation example 3:** | ...edge software in addition to this project manages the authorization and authentication of the different actors that are connectable and are been selected by the final owner of this software. |

## Use-case 6: more scheduling commands from more remote actors using all their own protocols

| **Description:** | Any actor want to provide the edge software with control schedules, but based on their own communication protocol. So the software in this project performing the scheduling IS the standardization on the behavior. Thereby there is almost full freedom to choose communication protocols best suited for the different actors in parallel. |
| --- | --- |
| **Example 1:** | A Charging Point Operator (CPO) wants to use OpenADR as a protocol to reduce the charging speed of a charger. |
| **Example 2:** | A DSO wants to use a 61850 MMS protocol to put the right limits to reduce infeed from wind-power during curtailment or redispatch processes. |
| **Example 3:** | A DSO wants to use a REST interface with 61850 semantics to support smaller customers with safe grid limits that can be integrated in their (home)automation. |
| **Example 4:** | All examples above and others can run in parallel without any conflict as long as the priority of the send schedules are programmed according to (national) policy. |
| **Expected value of this project:** | See earlier use-cases plus no actor is limited to use their own communication protocol that is optimal for their business. |
| **An implementation example 1:** | Using the core scheduling functionality of the edge software (this project) to run more schedules in parallel from more actors. |
| **An implementation example 2:** | Using the core protocol handling functionality of the FledgePower software (in this project) to connect the protocol of preference between WAN and LAN and in addition to the scheduling functionality software |
