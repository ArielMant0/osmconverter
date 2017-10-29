# OSM Converter 
Converts OpenStreetMap PBF-Files into a database with multiple LoDs that can be used in realtime-rendering software. This software was developed for windows.

### Functionality: 
 - two line simplification algorithms (Douglas-Peucker, Visvalingam-Whyatt) 
 - polygon-merging 
 - data-streaming 

### TODOs:  
 - handle large files 
 - handle data that needs to be assembled over multiple reading batches 

# Dependencies 
 * protobuf library (google)
