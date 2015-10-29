package dk.thoerup.osrmbinding;


public class ViarouteResult {

	public int status;

	public String startpoint;
	public String endpoint;
	public int totalTime;
	public int totalDistance;

	public ViarouteResult(int st, String start, String end, int time, int dist) {
		status = st;
		startpoint = start;
		endpoint = end;
		totalTime = time;
		totalDistance = dist;
	}

	@Override 
	public String toString() {
		return "ViarouteResult {" + startpoint + "->" + endpoint + ", time=" + totalTime + " dist=" + totalDistance + "}";
	}
}
