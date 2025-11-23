/**
 * @file
 * @brief     Handles state for all instances and strategies.
 * @details   State is stored in memory but also written to a file during live or demo trading so it can be recovered after a restart of the trading platform.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#ifndef INSTANCE_STATES_H_
#define INSTANCE_STATES_H_
#pragma once

#ifndef ASIRIKUY_DEFINES_H_
  #include "AsirikuyDefines.h"
#endif

#if !defined _WIN32 && !defined _WIN64
typedef unsigned int __time32_t;
#if !defined max && !defined min
  #define max fmax
  #define min fmin
#endif
#endif

typedef struct instanceState_t
{
  int           instanceId;
  __time32_t    lastRunTime;
  __time32_t    lastOrderUpdateTime;
  int           totalParameters;
  ParameterInfo parameterSpace[MAX_PARAMETERS];
  BOOL          isParameterSpaceLoaded;
  //double		predictDailyATR;
} InstanceState;

#ifdef __cplusplus
extern "C" {
#endif

/**
* Sets the folder in which to store instance states and initializes all the values in all instance state buffers.
*
* @param const char* folderPath
*   The folder that the instance states files are stored in.
*/
void initializeInstanceStates(const char* folderPath);

/**
* Loads the instance state from file.
*
* @param int instanceId
*   The ID of the instance state to be loaded.
*/
void loadInstanceState(int instanceId);

/**
* Get's the state of the specified instance.
*
* @param int instanceId
*   The ID of the instance state to be retrieved.
*
* @return InstanceState*
*   A pointer to the instance state.
*/
InstanceState* getInstanceState(int instanceId);

/**
* Checks if an instance has been previously run on the current bar.
*
* @param int instanceId
*   The ID of the instance to be queried.
*
* @param time_t barTime
*   The opening time of the current bar.
*
* @param BOOL isBackTesting
*   Set to TRUE if back testing, Or FALSE for live or demo trading.
*
* @return BOOL
*   TRUE if the instance has been run on the current bar; FALSE if it hasn't.
*/
BOOL hasInstanceRunOnCurrentBar(int instanceId, time_t barTime, BOOL isBackTesting);

/**
* Checks if an order has been opened on the current bar.
*
* @param StrategyParams* pParams
*   The structure containing all necessary parameters.
*
* @return BOOL
*   TRUE if an order has been opened on the current bar; FALSE if it hasn't.
*/
BOOL hasOrderOpenedOnCurrentBar(StrategyParams* pParams);

/**
* Stores the time the last order was created or modified.
*
* @param int instanceId
*   The ID of the instance to be queried.
*
* @param time_t updateTime
*   The current time.
*
* @param BOOL isBackTesting
*   Set to TRUE if back testing, Or FALSE for live or demo trading.
*
* @return time_t
*   The last time the order was updated or 0.
*/
time_t setLastOrderUpdateTime(int instanceId, time_t updateTime, BOOL isBackTesting);

/**
* Retrieves the time the last order was created or modified.
*
* @param int instanceId
*   The ID of the instance to be queried.
*
* @return time_t
*   The last time the order was updated or 0.
*/
time_t getLastOrderUpdateTime(int instanceId);

//int setPredictDailyATR(int instanceId, double dailyATR, BOOL isBackTesting);
//double getPredictDailyATR(int instanceId);

/**
 * Resets the internal state variables of an instance.
 *
 * @param int instanceId
 *   The ID of the instance to be reset.
 */
void resetInstanceState(int instanceId);

/**
* Gets the parameter space buffer for the specified instance
*
* @param int instanceId
*   The ID of the instance.
*
* @return ParameterInfo*
*   A pointer to the ParameterSpace array or NULL if it fails.
*/
//ParameterInfo* getParameterSpaceBuffer(int instanceId, int** ppTotalParameters);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INSTANCE_STATES_H_ */
