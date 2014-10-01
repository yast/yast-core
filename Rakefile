require "yast/rake"

Yast::Tasks.configuration do |conf|
  conf.obs_api = "https://api.suse.de/"
  conf.obs_target = "SLE_12"
  conf.obs_sr_project = "SUSE:SLE-12:Update:Test"
  conf.obs_project = "Devel:YaST:SLE-12"
  conf.skip_license_check << /testsuite/ #skip all testsuites
  conf.skip_license_check << /scr\/doc\/scr-arch.sda/ #skip binary file
  conf.skip_license_check << /.*/ #skip all temporary, as there is too much failures
end

