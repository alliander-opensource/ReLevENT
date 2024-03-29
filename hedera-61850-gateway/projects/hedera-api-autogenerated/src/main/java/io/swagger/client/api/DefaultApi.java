/*
 * Hedera - Hub for Energy Distribution and Excess Resource Allocation
 * <h2>Intended use</h2> <p>   This api is intended to be used by Market Participants who have a contract with the Grid Operator to receive additional capacity.    Schedules can be requested to receive capacity that can be used by the Market Participant. Once schedule with the provisioned capacity   is acquired the Market Participant is expected to live within those set point boundaries. If the Market Participant comes to the conclusion   that he needs more or less capacity he can updated his requested schedule. If the schedule is no longer required it is expected that the Market Participant   removes it. </p>  <h2>Authentication</h2> <p>   The api is secured with OAuth2. Once a contract is provided to the Market Participant credentials of grant type \"client_credentials\" will be provided.    The <strong>client_id</strong> and <strong>client_secret</strong> can be used to authenticate with <a href=\"https://auth.hedera.alliander.com/\">auth.hedera.alliander.com</a>. The bearer token can then be used in the Authentication header as follows <code>Authorization: Bearer &lt;token&gt;</code>. </p>  <h2>Versioning</h2> <p>   This API implements <b>MediaType-based</b> versioning. To request a specific version use the accept header, for example:   <code>Accept: application/vnd.hedera.v1+json</code>. If no version is specified you will request the latest version automatically.    Be aware that not providing a version can cause issues when breaking changes are released in the new version and is therefore not recommended.    When using and older version of the API you will received a Sunset header giving an indication of when support for that version will be removed in the future. </p>  <h2>Non breaking changes</h2> <p>   Within the current major version it is allowed to make non breaking changes in the same major version of the api. Non breaking changed are defined as follows: </p> <ul>   <li>Adding a endpoint</li>   <li>Adding a resource</li>   <li>Adding a optional field to a existing resource</li>   <li>Adding a parameter to a endpoint</li>   <li>Adding enums to fields that have a fallback (default) value</li> </ul>  <h2>Connectivity issues</h2> <p>   When experiencing connection problems with Hedera it is expected that the Market Participant falls back to its Firm Capacity specified in the contract with the Grid Operator. Reasoning behind this is that if we can not communicate anymore we run the risk of overloading the grid capacity limits. The grid must be protected and be as stable as possible at all times and when communication is not possible every Market Participant needs to fallback to its Firm Capacity limits. </p> 
 *
 * OpenAPI spec version: 1.0.0
 * Contact: support@hedera.alliander.com
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */

package io.swagger.client.api;

import io.swagger.client.ApiCallback;
import io.swagger.client.ApiClient;
import io.swagger.client.ApiException;
import io.swagger.client.ApiResponse;
import io.swagger.client.Configuration;
import io.swagger.client.Pair;
import io.swagger.client.ProgressRequestBody;
import io.swagger.client.ProgressResponseBody;

import com.google.gson.reflect.TypeToken;

import java.io.IOException;


import java.util.UUID;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class DefaultApi {
    private ApiClient apiClient;

    public DefaultApi() {
        this(Configuration.getDefaultApiClient());
    }

    public DefaultApi(ApiClient apiClient) {
        this.apiClient = apiClient;
    }

    public ApiClient getApiClient() {
        return apiClient;
    }

    public void setApiClient(ApiClient apiClient) {
        this.apiClient = apiClient;
    }

    /**
     * Build call for scheduleMRIDOptions
     * @param mRID  (required)
     * @param progressListener Progress listener
     * @param progressRequestListener Progress request listener
     * @return Call to execute
     * @throws ApiException If fail to serialize the request body object
     */
    public com.squareup.okhttp.Call scheduleMRIDOptionsCall(UUID mRID, final ProgressResponseBody.ProgressListener progressListener, final ProgressRequestBody.ProgressRequestListener progressRequestListener) throws ApiException {
        Object localVarPostBody = null;
        
        // create path and map variables
        String localVarPath = "/schedule/{mRID}"
            .replaceAll("\\{" + "mRID" + "\\}", apiClient.escapeString(mRID.toString()));

        List<Pair> localVarQueryParams = new ArrayList<Pair>();
        List<Pair> localVarCollectionQueryParams = new ArrayList<Pair>();

        Map<String, String> localVarHeaderParams = new HashMap<String, String>();

        Map<String, Object> localVarFormParams = new HashMap<String, Object>();

        final String[] localVarAccepts = {
            
        };
        final String localVarAccept = apiClient.selectHeaderAccept(localVarAccepts);
        if (localVarAccept != null) localVarHeaderParams.put("Accept", localVarAccept);

        final String[] localVarContentTypes = {
            
        };
        final String localVarContentType = apiClient.selectHeaderContentType(localVarContentTypes);
        localVarHeaderParams.put("Content-Type", localVarContentType);

        if(progressListener != null) {
            apiClient.getHttpClient().networkInterceptors().add(new com.squareup.okhttp.Interceptor() {
                @Override
                public com.squareup.okhttp.Response intercept(com.squareup.okhttp.Interceptor.Chain chain) throws IOException {
                    com.squareup.okhttp.Response originalResponse = chain.proceed(chain.request());
                    return originalResponse.newBuilder()
                    .body(new ProgressResponseBody(originalResponse.body(), progressListener))
                    .build();
                }
            });
        }

        String[] localVarAuthNames = new String[] {  };
        return apiClient.buildCall(localVarPath, "OPTIONS", localVarQueryParams, localVarCollectionQueryParams, localVarPostBody, localVarHeaderParams, localVarFormParams, localVarAuthNames, progressRequestListener);
    }
    
    @SuppressWarnings("rawtypes")
    private com.squareup.okhttp.Call scheduleMRIDOptionsValidateBeforeCall(UUID mRID, final ProgressResponseBody.ProgressListener progressListener, final ProgressRequestBody.ProgressRequestListener progressRequestListener) throws ApiException {
        // verify the required parameter 'mRID' is set
        if (mRID == null) {
            throw new ApiException("Missing the required parameter 'mRID' when calling scheduleMRIDOptions(Async)");
        }
        
        com.squareup.okhttp.Call call = scheduleMRIDOptionsCall(mRID, progressListener, progressRequestListener);
        return call;

        
        
        
        
    }

    /**
     * 
     * Returns the permitted communication options for a given endpoint
     * @param mRID  (required)
     * @throws ApiException If fail to call the API, e.g. server error or cannot deserialize the response body
     */
    public void scheduleMRIDOptions(UUID mRID) throws ApiException {
        scheduleMRIDOptionsWithHttpInfo(mRID);
    }

    /**
     * 
     * Returns the permitted communication options for a given endpoint
     * @param mRID  (required)
     * @return ApiResponse&lt;Void&gt;
     * @throws ApiException If fail to call the API, e.g. server error or cannot deserialize the response body
     */
    public ApiResponse<Void> scheduleMRIDOptionsWithHttpInfo(UUID mRID) throws ApiException {
        com.squareup.okhttp.Call call = scheduleMRIDOptionsValidateBeforeCall(mRID, null, null);
        return apiClient.execute(call);
    }

    /**
     *  (asynchronously)
     * Returns the permitted communication options for a given endpoint
     * @param mRID  (required)
     * @param callback The callback to be executed when the API call finishes
     * @return The request call
     * @throws ApiException If fail to process the API call, e.g. serializing the request body object
     */
    public com.squareup.okhttp.Call scheduleMRIDOptionsAsync(UUID mRID, final ApiCallback<Void> callback) throws ApiException {

        ProgressResponseBody.ProgressListener progressListener = null;
        ProgressRequestBody.ProgressRequestListener progressRequestListener = null;

        if (callback != null) {
            progressListener = new ProgressResponseBody.ProgressListener() {
                @Override
                public void update(long bytesRead, long contentLength, boolean done) {
                    callback.onDownloadProgress(bytesRead, contentLength, done);
                }
            };

            progressRequestListener = new ProgressRequestBody.ProgressRequestListener() {
                @Override
                public void onRequestProgress(long bytesWritten, long contentLength, boolean done) {
                    callback.onUploadProgress(bytesWritten, contentLength, done);
                }
            };
        }

        com.squareup.okhttp.Call call = scheduleMRIDOptionsValidateBeforeCall(mRID, progressListener, progressRequestListener);
        apiClient.executeAsync(call, callback);
        return call;
    }
    /**
     * Build call for scheduleOptions
     * @param progressListener Progress listener
     * @param progressRequestListener Progress request listener
     * @return Call to execute
     * @throws ApiException If fail to serialize the request body object
     */
    public com.squareup.okhttp.Call scheduleOptionsCall(final ProgressResponseBody.ProgressListener progressListener, final ProgressRequestBody.ProgressRequestListener progressRequestListener) throws ApiException {
        Object localVarPostBody = null;
        
        // create path and map variables
        String localVarPath = "/schedule";

        List<Pair> localVarQueryParams = new ArrayList<Pair>();
        List<Pair> localVarCollectionQueryParams = new ArrayList<Pair>();

        Map<String, String> localVarHeaderParams = new HashMap<String, String>();

        Map<String, Object> localVarFormParams = new HashMap<String, Object>();

        final String[] localVarAccepts = {
            
        };
        final String localVarAccept = apiClient.selectHeaderAccept(localVarAccepts);
        if (localVarAccept != null) localVarHeaderParams.put("Accept", localVarAccept);

        final String[] localVarContentTypes = {
            
        };
        final String localVarContentType = apiClient.selectHeaderContentType(localVarContentTypes);
        localVarHeaderParams.put("Content-Type", localVarContentType);

        if(progressListener != null) {
            apiClient.getHttpClient().networkInterceptors().add(new com.squareup.okhttp.Interceptor() {
                @Override
                public com.squareup.okhttp.Response intercept(com.squareup.okhttp.Interceptor.Chain chain) throws IOException {
                    com.squareup.okhttp.Response originalResponse = chain.proceed(chain.request());
                    return originalResponse.newBuilder()
                    .body(new ProgressResponseBody(originalResponse.body(), progressListener))
                    .build();
                }
            });
        }

        String[] localVarAuthNames = new String[] {  };
        return apiClient.buildCall(localVarPath, "OPTIONS", localVarQueryParams, localVarCollectionQueryParams, localVarPostBody, localVarHeaderParams, localVarFormParams, localVarAuthNames, progressRequestListener);
    }
    
    @SuppressWarnings("rawtypes")
    private com.squareup.okhttp.Call scheduleOptionsValidateBeforeCall(final ProgressResponseBody.ProgressListener progressListener, final ProgressRequestBody.ProgressRequestListener progressRequestListener) throws ApiException {
        
        com.squareup.okhttp.Call call = scheduleOptionsCall(progressListener, progressRequestListener);
        return call;

        
        
        
        
    }

    /**
     * 
     * Returns the permitted communication options for a given endpoint
     * @throws ApiException If fail to call the API, e.g. server error or cannot deserialize the response body
     */
    public void scheduleOptions() throws ApiException {
        scheduleOptionsWithHttpInfo();
    }

    /**
     * 
     * Returns the permitted communication options for a given endpoint
     * @return ApiResponse&lt;Void&gt;
     * @throws ApiException If fail to call the API, e.g. server error or cannot deserialize the response body
     */
    public ApiResponse<Void> scheduleOptionsWithHttpInfo() throws ApiException {
        com.squareup.okhttp.Call call = scheduleOptionsValidateBeforeCall(null, null);
        return apiClient.execute(call);
    }

    /**
     *  (asynchronously)
     * Returns the permitted communication options for a given endpoint
     * @param callback The callback to be executed when the API call finishes
     * @return The request call
     * @throws ApiException If fail to process the API call, e.g. serializing the request body object
     */
    public com.squareup.okhttp.Call scheduleOptionsAsync(final ApiCallback<Void> callback) throws ApiException {

        ProgressResponseBody.ProgressListener progressListener = null;
        ProgressRequestBody.ProgressRequestListener progressRequestListener = null;

        if (callback != null) {
            progressListener = new ProgressResponseBody.ProgressListener() {
                @Override
                public void update(long bytesRead, long contentLength, boolean done) {
                    callback.onDownloadProgress(bytesRead, contentLength, done);
                }
            };

            progressRequestListener = new ProgressRequestBody.ProgressRequestListener() {
                @Override
                public void onRequestProgress(long bytesWritten, long contentLength, boolean done) {
                    callback.onUploadProgress(bytesWritten, contentLength, done);
                }
            };
        }

        com.squareup.okhttp.Call call = scheduleOptionsValidateBeforeCall(progressListener, progressRequestListener);
        apiClient.executeAsync(call, callback);
        return call;
    }
}
