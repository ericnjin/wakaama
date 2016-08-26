#include "liblwm2m.h"
#include "lwm2mclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#include "../global.h"

#define LWM2M_TEMPERATURE_OBJECT_ID   3303
#define PRV_TEMPERATURE_SENSOR_UNITS    "Cel"

// Resource Id's:
#define RES_SENSOR_VALUE    5700
#define RES_SENSOR_UNITS    5701

#define VALUE_RES_SENSOR_VALUE              "-38.6"
#define VALUE_RES_SENSOR_UNITS              "Cel"

// static uint8_t prv_set_value(lwm2m_data_t * dataP,
//                              device_data_t * devDataP)
// {

// }
// typedef struct
// {
//     char     tempValue [10]; //"359.12345" frag=5, 9+1=10! degrees +\0
//     char     tempUnit  [10];
    
// } temper_data_t;

// static uint8_t prv_set_value(lwm2m_data_t * dataP,
//                              temper_data_t * tempDataP)

static uint8_t prv_set_value(lwm2m_data_t * dataP)
{
    // a simple switch structure is used to respond at the specified resource asked
    switch (dataP->id) {
    case RES_SENSOR_VALUE:
        lwm2m_data_encode_string(VALUE_RES_SENSOR_VALUE, dataP);
        return COAP_205_CONTENT ;

    case RES_SENSOR_UNITS:
        lwm2m_data_encode_string(VALUE_RES_SENSOR_UNITS, dataP);
        return COAP_205_CONTENT ;

    default:
        return COAP_404_NOT_FOUND ;
    }
}

// static uint8_t prv_device_read(uint16_t instanceId,
//                                int * numDataP,
//                                lwm2m_data_t ** dataArrayP,
//                                lwm2m_object_t * objectP)
// {



static uint8_t prv_temper_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    if (instanceId != 0) {
        return COAP_404_NOT_FOUND ;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0) {

        uint16_t resList[] = { RES_SENSOR_VALUE, RES_SENSOR_UNITS, };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        //*dataArrayP = lwm2m_tlv_new(nbRes);
        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR ;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++) {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do {
        result = prv_set_value((*dataArrayP) + i);
        //result = prv_set_value((*dataArrayP) + i, (temper_data_t*) (objectP->userData));
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT );

    return result;
}


// -----------------------------------------------------------------------
// Add Write operation for Observe Test for Temper Obj -Eric Kim, 2016.8
// -----------------------------------------------------------------------
static uint8_t prv_temper_write(uint16_t instanceId,
                         int numData,
                         lwm2m_data_t * dataArray,
                         lwm2m_object_t * objectP)
{
    prv_instance_t * targetP;
    int i;

    targetP = (prv_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    for (i = 0 ; i < numData ; i++)
    {
        switch (dataArray[i].id)
        {
        case 1:
        {
            int64_t value;

            if (1 != lwm2m_data_decode_int(dataArray + i, &value) || value < 0 || value > 0xFF)
            {
                return COAP_400_BAD_REQUEST;
            }
            targetP->test = (uint8_t)value;
        }
        break;
        case 2:
            return COAP_405_METHOD_NOT_ALLOWED;
        case 3:
            if (1 != lwm2m_data_decode_float(dataArray + i, &(targetP->dec)))
            {
                return COAP_400_BAD_REQUEST;
            }
            break;
        default:
            return COAP_404_NOT_FOUND;
        }
    }

    return COAP_204_CHANGED;
}


// static void prv_temperature_close(lwm2m_object_t * objectP) {
//     if (NULL != objectP->instanceList) {
//         lwm2m_free(objectP->instanceList);
//         objectP->instanceList = NULL;
//     }
// }
void free_object_temper(lwm2m_object_t * objectP)
{
    lwm2m_free(objectP->userData);
    lwm2m_list_free(objectP->instanceList);
    lwm2m_free(objectP);
}



lwm2m_object_t * get_object_temper() {
    /*
     * The get_object_tem function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * temperatureObj;

    temperatureObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != temperatureObj) {
        memset(temperatureObj, 0, sizeof(lwm2m_object_t));

        /*
         * It assigns his unique ID
         * The 3313 is the standard ID for the mandatory object "IPSO Accelerometer".
         */
        temperatureObj->objID = LWM2M_TEMPERATURE_OBJECT_ID;

        /*
         * there is only one instance of accelerometer on the Application Board for mbed NXP LPC1768.
         *
         */
        temperatureObj->instanceList = (lwm2m_list_t *) lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != temperatureObj->instanceList) {
            memset(temperatureObj->instanceList, 0, sizeof(lwm2m_list_t));
        } else {
            lwm2m_free(temperatureObj);
            return NULL;
        }

        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        temperatureObj->readFunc = prv_temper_read;
        temperatureObj->writeFunc = prv_temper_write;
        temperatureObj->executeFunc = NULL;
        //temperatureObj->closeFunc = free_object_temper;
        temperatureObj->userData = NULL;

        // if (NULL != temperatureObj->userData)
        // {
        //     temper_data_t* data = (location_data_t*)temperatureObj->userData;
        //     strcpy (data->tempValue,     "27.98");  // Mount Everest :)
        //     strcpy (data->tempUnit,    "Far");
            
        // }
        // else
        // {
        //     lwm2m_free(temperatureObj);
        //     temperatureObj = NULL;
        // }
    }


    return temperatureObj;
}


