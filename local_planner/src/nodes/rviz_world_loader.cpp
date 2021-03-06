#include "rviz_world_loader.h"

// extraction operators
void operator >> (const YAML::Node& node, Eigen::Vector3f& v) {
   v.x() = node[0].as<float>();
   v.y() = node[1].as<float>();
   v.z() = node[2].as<float>();
}

void operator >> (const YAML::Node& node, Eigen::Vector4f& v) {
   v.x() = node[0].as<float>();
   v.y() = node[1].as<float>();
   v.z() = node[2].as<float>();
   v.w() = node[3].as<float>();
}

void operator >> (const YAML::Node& node, world_object& item) {
   item.type = node["type"].as<std::string>();
   item.name = node["name"].as<std::string>();
   item.frame_id = node["frame_id"].as<std::string>();
   item.mesh_resource = node["mesh_resource"].as<std::string>();
   node["position"] >> item.position;
   node["orientation"] >> item.orientation;
   node["scale"] >> item.scale;
}

int resolveUri(std::string& uri){

  // Iterate through all locations in GAZEBO_MODEL_PATH
  char* gazebo_model_path = getenv("GAZEBO_MODEL_PATH");
  char* home = getenv("HOME");
  uri = uri.substr(7, std::string::npos);
  std::stringstream all_locations(gazebo_model_path, std::ios_base::app | std::ios_base::out | std::ios_base::in);
  all_locations << ":" << home << "/.gazebo/models";
  std::string current_location;
  while (getline(all_locations, current_location, ':'))
  {
    struct stat s;
    std::string temp = current_location + uri;
    if (stat(temp.c_str(), &s) == 0)
    {
      if (s.st_mode & S_IFREG)  //this path describes a file
      {
        uri =  "file://" + current_location + uri;
        return 0;
      }
    }
  }
  return 1;
}

int visualizeRVIZWorld(const std::string& world_path, visualization_msgs::MarkerArray& marker_array){

  std::ifstream fin(world_path);
  YAML::Node doc = YAML::Load(fin);
  int object_counter = 0;

  for(YAML::const_iterator it = doc.begin(); it != doc.end(); ++it) {
    const YAML::Node& node = *it;
    world_object item;
    node >> item;
    object_counter ++;

    //convert object to marker
    visualization_msgs::Marker m;
    m.header.frame_id = item.frame_id;
    m.header.stamp = ros::Time::now();

    if(item.type == "mesh"){
      if(item.mesh_resource.find("model://")!=std::string::npos){
        if(resolveUri(item.mesh_resource)){
          ROS_ERROR("RVIZ world loader could not find model");
          return 1;
        }
      }
      m.type = visualization_msgs::Marker::MESH_RESOURCE;
      m.mesh_resource = item.mesh_resource;
      m.mesh_use_embedded_materials = true;
    }else if(item.type == "cube"){
      m.type = visualization_msgs::Marker::CUBE;
      m.color.a = 0.9;
      m.color.r = 0.5;
      m.color.g = 0.5;
      m.color.b = 0.5;
   	}else if(item.type == "sphere"){
      m.type = visualization_msgs::Marker::SPHERE;
      m.color.a = 0.9;
      m.color.r = 0.5;
      m.color.g = 0.5;
      m.color.b = 0.5;
    }else if(item.type == "cylinder"){
      m.type = visualization_msgs::Marker::CYLINDER;
      m.color.a = 0.9;
      m.color.r = 0.5;
      m.color.g = 0.5;
      m.color.b = 0.5;
    }else{
      ROS_ERROR("RVIZ world loader invalid object type in yaml file");
      return 1;
    }

    m.scale.x = item.scale.x();
    m.scale.y = item.scale.y();
    m.scale.z = item.scale.z();
    m.pose.position.x = item.position.x();
    m.pose.position.y = item.position.y();
    m.pose.position.z = item.position.z();
    m.pose.orientation.x = item.orientation.x();
    m.pose.orientation.y = item.orientation.y();
    m.pose.orientation.z = item.orientation.z();
    m.pose.orientation.w = item.orientation.w();
    m.id = object_counter;
    m.lifetime = ros::Duration();
    m.action = visualization_msgs::Marker::ADD;
    marker_array.markers.push_back(m);
  }

  if(object_counter != marker_array.markers.size()){
    ROS_ERROR("Could not display all world objects");
  }

   ROS_INFO("Successfully loaded rviz world");
   return 0;
}
