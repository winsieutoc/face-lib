#include "StorageUtils.hpp"

StorageUtils::StorageUtils() {
  endpoint = "192.168.100.5:9000";
  accessId = "admin";
  secretKey = "abc123456";
}

void StorageUtils::uploadFile(cv::Mat &img, std::string _fileName,
                              std::string camid) {
  bool debug = false;
  std::vector<unsigned char> imgBuffer;
  cv::imencode(".jpg", img, imgBuffer);

  // Buffer to String with pointer/length
  unsigned char *imgBuffPtr = &imgBuffer[0];
  long imgBuffLength = imgBuffer.size();

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    const Aws::String bucketName = "faces";

    const Aws::String fileName = _fileName.c_str();
    Aws::Client::ClientConfiguration config;
    config.scheme = Aws::Http::Scheme::HTTP;
    config.connectTimeoutMs = 30000;
    config.requestTimeoutMs = 30000;
    config.endpointOverride = this->endpoint; // customized address
    Aws::S3::S3Client *s3Client = new Aws::S3::S3Client(
        Aws::Auth::AWSCredentials(this->accessId, this->secretKey), config,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    //    auto outcome = s3_client->ListBuckets();

    Aws::S3::Model::PutObjectRequest objectRequest;
    objectRequest.WithBucket(bucketName);

    // Binary files must also have the std::ios_base::bin flag or'ed in
    std::string contentLength = std::to_string(imgBuffLength);
    std::shared_ptr<Aws::IOStream> body =
        std::shared_ptr<Aws::IOStream>(new boost::interprocess::bufferstream(
            (char *)imgBuffPtr, imgBuffLength));

    Aws::S3::Model::PutObjectRequest putObjectRequest;
    putObjectRequest.WithBucket(bucketName).WithKey(fileName).SetBody(body);

    auto putObjectOutcome = s3Client->PutObject(putObjectRequest);

    if (putObjectOutcome.IsSuccess()) {
      if (debug)
        std::cout << "Done!" << std::endl;
    } else {
      if (debug)
        std::cout << "PutObject error: "
                  << putObjectOutcome.GetError().GetExceptionName() << " "
                  << putObjectOutcome.GetError().GetMessage() << std::endl;
    }
  }
  Aws::ShutdownAPI(options);
}
