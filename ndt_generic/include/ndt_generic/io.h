#pragma once

#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Eigenvalues>
#include <angles/angles.h>
#include <iomanip>
#include <iostream>
#include <ndt_generic/utils.h>
#include <pcl/io/pcd_io.h>
#include "ros/time.h"
#include "ros/ros.h"
#include "string.h"
namespace ndt_generic {

// Load the evaluation files that are generated by the fuser, <timestamp> x y x qx qy qz qw.
std::vector<Eigen::Affine3d> loadAffineFromEvalFile(const std::string &fileName) {
  std::vector<Eigen::Affine3d> ret;
  std::string line;
  std::ifstream myfile (fileName.c_str());
  if (myfile.is_open())
  {
    while ( getline (myfile,line) )
    {
      double time, x, y, z, qx, qy, qz, qw;
      std::istringstream ss(line);
      ss >> time >> x >> y >> z >> qx >> qy >> qz >> qw;
      ret.push_back(Eigen::Translation3d(x,y,z)*Eigen::Quaterniond(qw, qx, qy, qz));
    }
    myfile.close();
  }
  else {
    std::cout << "Unable to open file : " << fileName << std::endl;
  } 
  
  return ret;
}

// Load timestamps from the evaluation files that are generated by the fuser...
std::vector<double> loadTimeStampFromEvalFile(const std::string &fileName) {
  std::vector<double> ret;
  std::string line;
  std::ifstream myfile (fileName.c_str());
  if (myfile.is_open())
  {
    while ( getline (myfile,line) )
    {
      double time, x, y, z, qx, qy, qz, qw;
      std::istringstream ss(line);
      ss >> time >> x >> y >> z >> qx >> qy >> qz >> qw;
      ret.push_back(time);
   }
   myfile.close();
 }
  else std::cout << "Unable to open file : " << fileName << std::endl;; 
  
  return ret;
}


// Useful to load a set of files
void loadCloud(const std::string &base_name_pcd, int counter, pcl::PointCloud<pcl::PointXYZ> &cloud) {
    std::string pcd_file = base_name_pcd + std::string("cloud") + ndt_generic::toString(counter) + std::string(".pcd");
    std::cout << "loading : " << pcd_file << std::endl;
    pcl::io::loadPCDFile<pcl::PointXYZ>(pcd_file, cloud);
}


void saveAffine3dRPY(const std::string &filename, const Eigen::Affine3d &T) {
    std::ofstream ofs;
    ofs.open(filename.c_str());
    if (!ofs.is_open())
        return;
    ofs << ndt_generic::affine3dToStringRPY(T) << std::endl;
    ofs.close();
}

// Save the evaluation files that are generated by the fuser, <timestamp> x y x qx qy qz qw.
// Timesamps are set to the index.
void saveAffineToEvalFile(const std::string &filename, const std::vector<Eigen::Affine3d> &Ts) {
    std::ofstream ofs;
    ofs.open(filename.c_str());
    if (!ofs.is_open())
        return;
    for (int i = 0; i < Ts.size(); i++) {
        ofs << i << " " << ndt_generic::transformToEvalString(Ts[i]);
    }
    ofs.close();
}


std::vector<double> loadDoubleVecTextFile(const std::string &fileName) {

   std::vector<double> vec;
   std::ifstream ifs;
   ifs.open(fileName.c_str());
   if (!ifs.is_open()) {
     std::cerr << __FILE__ << ":" << __LINE__ << " cannot open file : " << fileName << std::endl;
   }
    
   while (!ifs.eof())
     {
       std::string line;
       getline(ifs, line);
       
       double val;
       if (sscanf(line.c_str(), "%lf",
		  &val) == 1)
	 {
	   vec.push_back(val);
	 }
     }
   return vec;
 }

 void saveDoubleVecTextFile(const std::vector<double> &vec, const std::string &fileName)
 {
   std::stringstream st;
   st << fileName;
   std::string file_name = st.str();
   std::ofstream ofs(file_name.c_str());
   
   for (unsigned int i = 0; i < vec.size(); i++)
     {
       ofs << vec[i] << std::endl;
     }  
   ofs.close();
 }
 class CreateEvalFiles{
 public:
   CreateEvalFiles(const std::string &output_dir_name, const std::string &base_name, bool enable=true){
      output_dir_name_=output_dir_name;
      std::cout<<"output:"<<output_dir_name_<<std::endl;
     if(output_dir_name_.length()>0 &&output_dir_name_[output_dir_name_.length()-1]!='/')
       output_dir_name_+='/';

     std::cout<<output_dir_name_<<std::endl;
     base_name_=base_name;
     enable_=enable;
     CreateOutputFiles();
   }

   void CreateOutputFiles(){
     if(!enable_)
       return;

     std::string filename;
     {
       filename =output_dir_name_ + base_name_ + std::string("_gt.txt");
       gt_file.open(filename.c_str());
     }
     {
       filename =output_dir_name_ + base_name_ + std::string("_est.txt");
       est_file.open(filename.c_str());
     }
     {
       filename = output_dir_name_ + base_name_ + std::string("_sensorpose_est.txt");
       sensorpose_est_file.open(filename.c_str());
     }
     {
       filename = output_dir_name_ + base_name_ + std::string("_odom.txt");
       odom_file.open(filename.c_str());
     }
     if (!gt_file.is_open() || !est_file.is_open() || !odom_file.is_open())
     {
       std::cout<<"Error opening evaluation output files at path:"<<std::endl;
       std::cout<<filename<<std::endl;
       exit(0);
     }
     else
       return;
   }
   void Write(const ros::Time frame_time ,const Eigen::Affine3d &Tgtbase,const Eigen::Affine3d &Todombase,const Eigen::Affine3d &Tfuserpose,const Eigen::Affine3d &sensoroffset){
     if(!enable_)
       return;
     std::cout<<"writing results  to file"<<std::endl;
     gt_file << frame_time << " " << transformToEvalString(Tgtbase);
     odom_file << frame_time << " " << transformToEvalString(Todombase);
     est_file << frame_time << " " << transformToEvalString(Tfuserpose);
     sensorpose_est_file << frame_time << " " << transformToEvalString(Tfuserpose * sensoroffset);
   }

   void Close(){
     if(!enable_)
       return;
     gt_file.close();
     odom_file.close();
     est_file.close();
     sensorpose_est_file.close();
   }

 private:
   bool enable_;
   std::string output_dir_name_,base_name_;
   std::ofstream gt_file, odom_file, est_file, sensorpose_est_file; //output files
 };



} // namespace
