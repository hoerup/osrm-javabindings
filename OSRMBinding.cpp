#include "dk_thoerup_osrmbinding_OSRMBinding.h"

#include <iostream>
#include <exception>

#include <osrm/osrm.hpp>
#include <osrm/json_container.hpp>
#include <osrm/libosrm_config.hpp>
#include <osrm/route_parameters.hpp>



jfieldID getPtrFieldId(JNIEnv * env, jobject obj)
{
    static jfieldID ptrFieldId = 0;

    if (!ptrFieldId)
    {
        jclass c = env->GetObjectClass(obj);
        ptrFieldId = env->GetFieldID(c, "ptr", "J");
        env->DeleteLocalRef(c);
    }

    return ptrFieldId;
}

std::vector<std::pair<double,double>> decodePairArray(JNIEnv * env,  jobjectArray objArray)
{

    static jclass cls;
    static jfieldID fieldLatitude;
    static jfieldID fieldLongitude;

    std::vector<std::pair<double,double>> result;

    int len = env->GetArrayLength(objArray);
    for (int i=0; i<len; i++)
    {
        double lat, lng;

        jobject obj = (jobject) env->GetObjectArrayElement(objArray, i);

        if (cls == NULL) {
            cls = env->GetObjectClass(obj);
            fieldLatitude = env->GetFieldID(cls, "latitude", "D");
            fieldLongitude = env->GetFieldID(cls, "longitude", "D");
        }


        lat = env->GetDoubleField(obj, fieldLatitude);
        lng = env->GetDoubleField(obj, fieldLongitude);
        result.push_back( std::pair<double,double>(lat,lng) );
    }

    return result;
}


jobjectArray table2jniarray(JNIEnv *env, float **input, const int size) {
    jobjectArray myReturnable2DArray;

    try {

        jclass floatArrayClass = env->FindClass( "[F");


        myReturnable2DArray  = env->NewObjectArray((jsize) size, floatArrayClass, NULL);

        for (unsigned int i = 0; i < size; i++)
        {
            jfloatArray floatArray = env->NewFloatArray( size);
            env->SetFloatArrayRegion( floatArray, (jsize) 0, (jsize) size, (jfloat*) input[i]);
            env->SetObjectArrayElement( myReturnable2DArray, (jsize) i, floatArray);
            env->DeleteLocalRef(floatArray);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }


    return myReturnable2DArray;
}


jobject createViarouteResult(JNIEnv *env, osrm::json::Object &json_result)
{

	int status = (int)  json_result.values["status"].get<osrm::json::Number>().value;
	osrm::json::Object route_summary = json_result.values["route_summary"].get<osrm::json::Object>();
	std::string start = route_summary.values["start_point"].get<osrm::json::String>().value;
	std::string end = route_summary.values["end_point"].get<osrm::json::String>().value;
	int time = (int)  route_summary.values["total_time"].get<osrm::json::Number>().value;
	int distance = (int)  route_summary.values["total_distance"].get<osrm::json::Number>().value;

    jclass cls = env->FindClass( "dk/thoerup/osrmbinding/ViarouteResult");
    if (cls == NULL) {
        std::cerr<< "Cant find class";
        return NULL;
    }
    jmethodID methodId = env->GetMethodID(cls, "<init>", "(ILjava/lang/String;Ljava/lang/String;II)V");
    if (methodId == NULL) {
        std::cerr<< "Cant find constructor";
        return NULL;
    }
	jstring startJstring = env->NewStringUTF( start.c_str() );
	jstring endJstring = env->NewStringUTF( end.c_str() );

    jobject result = env->NewObject(cls, methodId, status, startJstring, endJstring, time, distance);
    return result;

}




JNIEXPORT void JNICALL Java_dk_thoerup_osrmbinding_OSRMBinding_init(JNIEnv * env, jobject obj, jboolean sharedmem, jstring path) {



    libosrm_config lib_config;
	lib_config.max_locations_distance_table = 110;

	if (sharedmem) {
	    lib_config.use_shared_memory = true;
    	lib_config.server_paths["base"] = "";
	} else {
    	const char *nativePath = env->GetStringUTFChars(path, JNI_FALSE);
	    lib_config.use_shared_memory = false;
    	lib_config.server_paths["base"] = nativePath;
	}



    try {
        std::cout << "starting OSRM engine" << std::endl;

        OSRM* routing_machine = new OSRM(lib_config);

        env->SetLongField(obj, getPtrFieldId(env, obj), (jlong) routing_machine );

    }
    catch (std::exception &current_exception)
    {
        std::cerr << "caught exception: " << current_exception.what() << std::endl;
		env->ThrowNew( env->FindClass("java/lang/Exception"), current_exception.what() );
    }

}

JNIEXPORT void JNICALL Java_dk_thoerup_osrmbinding_OSRMBinding_destroy(JNIEnv *env, jobject obj)
{
    OSRM* routing_machine = (OSRM *) env->GetLongField(obj, getPtrFieldId(env, obj));
    delete routing_machine;
}



JNIEXPORT jobjectArray JNICALL Java_dk_thoerup_osrmbinding_OSRMBinding_table(JNIEnv *env, jobject obj, jobjectArray arr)
{

    OSRM* routing_machine = (OSRM *) env->GetLongField(obj, getPtrFieldId(env, obj));

    try {

        std::vector<std::pair<double,double>> pairArray = decodePairArray(env, arr);
        const int size = pairArray.size();

        RouteParameters route_parameters;
        route_parameters.service = "table";


        for (int i=i; i<pairArray.size(); i++) {
            std::pair<double,double> current = pairArray.at(i);
            double lat = current.first;
            double lng = current.second;

            route_parameters.coordinates.emplace_back(lat * COORDINATE_PRECISION,
                    lng * COORDINATE_PRECISION);

        }
        osrm::json::Object json_result;
        const int result_code = routing_machine->RunQuery(route_parameters, json_result);

        osrm::json::Array distance_table = json_result.values["distance_table"].get<osrm::json::Array>();

        float** table = new float*[size];
        for(int i = 0; i < size; ++i) {
            table[i] = new float[size];
        }

        // loop distance table rows
        int i = 0;
        for(osrm::json::Value row : distance_table.values)
        {
            int j=0;
            for(osrm::json::Value cell : row.get<osrm::json::Array>().values)
            {
                float cellValue = (float)cell.get<osrm::json::Number>().value;
                table[i][j] = cellValue;
                j++;
            }
            i++;
        }


        jobjectArray result = table2jniarray(env, table, size);

        for(int i = 0; i < size; ++i) {
            delete table[i];
        }
        delete table;

        return result;

    }
    catch (std::exception &current_exception)
    {
        std::cerr << "caught exception: " << current_exception.what();
		env->ThrowNew( env->FindClass("java/lang/Exception"), current_exception.what() );

    }
}

JNIEXPORT jobject JNICALL Java_dk_thoerup_osrmbinding_OSRMBinding_viaRoute(JNIEnv *env, jobject obj, jobjectArray arr)
{

    OSRM* routing_machine = (OSRM *) env->GetLongField(obj, getPtrFieldId(env, obj));

    try {

        std::vector<std::pair<double,double>> pairArray = decodePairArray(env, arr);

        RouteParameters route_parameters;
        route_parameters.zoom_level = 18;           // no generalization
        route_parameters.print_instructions = false; // turn by turn instructions
        route_parameters.alternate_route = false;    // get an alternate route, too
        route_parameters.geometry = false;           // retrieve geometry of route
        route_parameters.compression = true;        // polyline encoding
        route_parameters.check_sum = 0;            // see wiki
        route_parameters.service = "viaroute";      // that's routing
        route_parameters.output_format = "json";
        route_parameters.jsonp_parameter = ""; // set for jsonp wrapping
        route_parameters.language = "";        // unused atm
        // route_parameters.hints.push_back(); // see wiki, saves I/O if done properly

        for (int i=i; i<pairArray.size(); i++) {
            std::pair<double,double> current = pairArray.at(i);
            double lat = current.first;
            double lng = current.second;

            route_parameters.coordinates.emplace_back(lat * COORDINATE_PRECISION,
                    lng * COORDINATE_PRECISION);

        }

        osrm::json::Object json_result;
        const int result_code = routing_machine->RunQuery(route_parameters, json_result);

        return createViarouteResult(env, json_result);


    }
    catch (std::exception &current_exception)
    {
        std::cerr << "caught exception: " << current_exception.what();
		env->ThrowNew( env->FindClass("java/lang/Exception"), current_exception.what() );

    }

}


