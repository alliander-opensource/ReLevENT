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

package io.swagger.client.model;

import java.util.Objects;
import java.util.Arrays;
import com.google.gson.TypeAdapter;
import com.google.gson.annotations.JsonAdapter;
import com.google.gson.annotations.SerializedName;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonWriter;
import io.swagger.client.model.Issue;
import io.swagger.v3.oas.annotations.media.Schema;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.threeten.bp.OffsetDateTime;
/**
 * A descriptive server error to provide information about the call you made and what error occurred.
 */
@Schema(description = "A descriptive server error to provide information about the call you made and what error occurred.")
@javax.annotation.Generated(value = "io.swagger.codegen.v3.generators.java.JavaClientCodegen", date = "2023-11-15T12:54:05.868886500Z[GMT]")

public class ServerError {
  @SerializedName("requestId")
  private String requestId = null;

  @SerializedName("timestamp")
  private OffsetDateTime timestamp = null;

  /**
   * The method that you just called.
   */
  @JsonAdapter(MethodEnum.Adapter.class)
  public enum MethodEnum {
    @SerializedName("DELETE")
    DELETE("DELETE"),
    @SerializedName("GET")
    GET("GET"),
    @SerializedName("HEAD")
    HEAD("HEAD"),
    @SerializedName("OPTIONS")
    OPTIONS("OPTIONS"),
    @SerializedName("PATCH")
    PATCH("PATCH"),
    @SerializedName("POST")
    POST("POST"),
    @SerializedName("PUT")
    PUT("PUT");

    private String value;

    MethodEnum(String value) {
      this.value = value;
    }
    public String getValue() {
      return value;
    }

    @Override
    public String toString() {
      return String.valueOf(value);
    }
    public static MethodEnum fromValue(String input) {
      for (MethodEnum b : MethodEnum.values()) {
        if (b.value.equals(input)) {
          return b;
        }
      }
      return null;
    }
    public static class Adapter extends TypeAdapter<MethodEnum> {
      @Override
      public void write(final JsonWriter jsonWriter, final MethodEnum enumeration) throws IOException {
        jsonWriter.value(String.valueOf(enumeration.getValue()));
      }

      @Override
      public MethodEnum read(final JsonReader jsonReader) throws IOException {
        Object value = jsonReader.nextString();
        return MethodEnum.fromValue((String)(value));
      }
    }
  }  @SerializedName("method")
  private MethodEnum method = null;

  @SerializedName("path")
  private String path = null;

  @SerializedName("status")
  private Integer status = null;

  @SerializedName("issues")
  private List<Issue> issues = new ArrayList<Issue>();

   /**
   * A traceable id of the request.
   * @return requestId
  **/
  @Schema(example = "bea874a6-9", required = true, description = "A traceable id of the request.")
  public String getRequestId() {
    return requestId;
  }

   /**
   * ISO-8601 notated date time when the error occurred.
   * @return timestamp
  **/
  @Schema(example = "2022-12-06T09:00Z", required = true, description = "ISO-8601 notated date time when the error occurred.")
  public OffsetDateTime getTimestamp() {
    return timestamp;
  }

   /**
   * The method that you just called.
   * @return method
  **/
  @Schema(example = "GET", required = true, description = "The method that you just called.")
  public MethodEnum getMethod() {
    return method;
  }

   /**
   * Path of the endpoint you just called.
   * @return path
  **/
  @Schema(example = "/schedule/c033d9b1-330a-49a7-b88c-e26536725cd8", required = true, description = "Path of the endpoint you just called.")
  public String getPath() {
    return path;
  }

   /**
   * Response status of your request.
   * @return status
  **/
  @Schema(example = "500", required = true, description = "Response status of your request.")
  public Integer getStatus() {
    return status;
  }

   /**
   * Lists all the issues.
   * @return issues
  **/
  @Schema(required = true, description = "Lists all the issues.")
  public List<Issue> getIssues() {
    return issues;
  }


  @Override
  public boolean equals(java.lang.Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }
    ServerError serverError = (ServerError) o;
    return Objects.equals(this.requestId, serverError.requestId) &&
        Objects.equals(this.timestamp, serverError.timestamp) &&
        Objects.equals(this.method, serverError.method) &&
        Objects.equals(this.path, serverError.path) &&
        Objects.equals(this.status, serverError.status) &&
        Objects.equals(this.issues, serverError.issues);
  }

  @Override
  public int hashCode() {
    return Objects.hash(requestId, timestamp, method, path, status, issues);
  }


  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append("class ServerError {\n");
    
    sb.append("    requestId: ").append(toIndentedString(requestId)).append("\n");
    sb.append("    timestamp: ").append(toIndentedString(timestamp)).append("\n");
    sb.append("    method: ").append(toIndentedString(method)).append("\n");
    sb.append("    path: ").append(toIndentedString(path)).append("\n");
    sb.append("    status: ").append(toIndentedString(status)).append("\n");
    sb.append("    issues: ").append(toIndentedString(issues)).append("\n");
    sb.append("}");
    return sb.toString();
  }

  /**
   * Convert the given object to string with each line indented by 4 spaces
   * (except the first line).
   */
  private String toIndentedString(java.lang.Object o) {
    if (o == null) {
      return "null";
    }
    return o.toString().replace("\n", "\n    ");
  }

}