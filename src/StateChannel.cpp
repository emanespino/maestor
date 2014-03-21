/*
 * StateChannel.cpp
 *
 *  Created on: Oct 17, 2013
 *      Author: maestro
 */

#include "StateChannel.h"

const char *StateChannel::urdf_joint_names[] = {
        "WST", "NKY", "NK1", "NK2",
        "LSP", "LSR", "LSY", "LEP", "LWY", "LWR", "LWP",
        "RSP", "RSR", "RSY", "REP", "RWY", "RWR", "RWP",
        "UNUSED1",
        "LHY", "LHR", "LHP", "LKP", "LAP", "LAR",
        "UNUSED2",
        "RHY", "RHR", "RHP", "RKP", "RAP", "RAR",
        "RF1", "RF2", "RF3", "RF4", "RF5",
        "LF1", "LF2", "LF3", "LF4", "LF5",
        "unknown1", "unknown2", "unknown3", "unknown4", "unknown5", "unknown6", "unknown7", "unknown8"};

int StateChannel::indexLookup(string &joint) {
    if (joint.length() != 3)
        return -1;
    int best_match = -1;

    for (int i = 0; i < HUBO_JOINT_COUNT; i++) {
        if (strcmp(joint.c_str(), urdf_joint_names[i]) == 0)
            best_match = i;
    }
    return best_match;
}

StateChannel::StateChannel() {
    errored = false;
    int r = ach_open(&huboStateChannel, HUBO_CHAN_STATE_NAME, NULL);
    if (ACH_OK != r && ACH_MISSED_FRAME != r && ACH_STALE_FRAMES != r){
        cout << "Error! State Channel failed with state " << r << endl;
        errored = true;
    }
}

StateChannel::~StateChannel() { }

void StateChannel::load(){
    if (errored)
        return;

    State temp;
    memset( &temp, 0, sizeof(temp));
    size_t fs;
    int r = ach_get(&huboStateChannel, &temp, sizeof(temp), &fs, NULL, ACH_O_LAST);

    if(ACH_OK != r && ACH_MISSED_FRAME != r && ACH_STALE_FRAMES != r) {
        cout << "Error! State Channel failed with state " << r << endl;
        errored = true;
        return;
    } else if (ACH_STALE_FRAMES != r){
        if (sizeof(temp) != fs) {
            cout << "Error! File size inconsistent with state struct! fs = " << fs << " sizeof currentReference: " << sizeof(temp) << endl;
            errored = true;
            return;
        }
    } else
        return;
    currentReference = temp;
}

bool StateChannel::getMotorProperty(string &name, PROPERTY property, double &result){
    if (errored)
        return false;

    int index = indexLookup(name);
    if (index == -1)
        return false;

    switch (property){
    case POSITION:
        result = currentReference.joint[index].pos;
        break;
    case GOAL:
        result = currentReference.joint[index].ref;
        break;
    case VELOCITY:
        result = currentReference.joint[index].vel;
        break;
    case TEMPERATURE:
        result = currentReference.joint[index].tmp;
        break;
    case CURRENT:
        result = currentReference.joint[index].cur;
        break;
    case ENABLED:
        result = currentReference.joint[index].active;
        break;
    case HOMED:
        result = currentReference.status[index].homeFlag;
        break;
    case ERRORED:
        return false; // currently unsupported;
        //break; // unreachable code :D
    case JAM_ERROR:
        result = currentReference.status[index].jam;
        break;
    case PWM_SATURATED_ERROR:
        result = currentReference.status[index].pwmSaturated;
        break;
    case BIG_ERROR:
        result = currentReference.status[index].bigError;
        break;
    case ENC_ERROR:
        result = currentReference.status[index].encError;
        break;
    case DRIVE_FAULT_ERROR:
        result = currentReference.status[index].driverFault;
        break;
    case POS_MIN_ERROR:
        result = currentReference.status[index].posMinError;
        break;
    case POS_MAX_ERROR:
        result = currentReference.status[index].posMaxError;
        break;
    case VELOCITY_ERROR:
        result = currentReference.status[index].velError;
        break;
    case ACCELERATION_ERROR:
        result = currentReference.status[index].accError;
        break;
    case TEMP_ERROR:
        result = currentReference.status[index].tempError;
        break;
    default:
        return false;
    }
    return true;
}

bool StateChannel::getIMUProperty(string &name, PROPERTY property, double& result){
    if (errored)
        return false;

    int index = -1;
    if (strcmp(name.c_str(), "IMU") == 0)
        index = BODY_IMU;
    else if (strcmp(name.c_str(), "LAI") == 0)
        index = LEFT_IMU;
    else if (strcmp(name.c_str(), "RAI") == 0)
        index = RIGHT_IMU;

    if (index == -1)
        return false;

    switch (property){
    case X_ACCEL:
        result = currentReference.imu[index].a_x;
        break;
    case Y_ACCEL:
        result = currentReference.imu[index].a_y;
        break;
    case Z_ACCEL:
        result = currentReference.imu[index].a_z;
        break;
    case X_ROTAT:
        result = currentReference.imu[index].w_x;
        break;
    case Y_ROTAT:
        result = currentReference.imu[index].w_y;
        break;
    default:
        return false;
    }
    return true;
}

bool StateChannel::getFTProperty(string &name, PROPERTY property, double& result){
    if (errored)
        return false;

    int index = -1;
    if (strcmp(name.c_str(), "LAT") == 0)
        index = FT_LA;
    else if (strcmp(name.c_str(), "RAT") == 0)
        index = FT_RA;
    else if (strcmp(name.c_str(), "LWT") == 0)
        index = FT_LW;
    else if (strcmp(name.c_str(), "RWT") == 0)
        index = FT_RW;

    if (index == -1)
        return false;

    switch (property){
    case M_X:
        result = currentReference.ft[index].m_x;
        break;
    case M_Y:
        result = currentReference.ft[index].m_y;
        break;
    case F_Z:
        result = currentReference.ft[index].f_z;
        break;
    default:
        return false;
    }
    return true;
}
