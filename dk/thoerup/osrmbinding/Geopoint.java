package dk.thoerup.osrmbinding;


public class Geopoint {

	public double latitude;
	public double longitude;

	public Geopoint(double lat, double lng) {
		latitude = lat;
		longitude = lng;
	}

	@Override 
	public String toString() {
		return "Geopoint {" + latitude + ", " + longitude + "}";
	}
}
