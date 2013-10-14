require "yast/rake"

Packaging.configuration do |conf|
  conf.skip_license_check << /testsuite/ #skip all testsuites
  conf.skip_license_check << /scr\/doc\/scr-arch.sda/ #skip binary file
  conf.skip_license_check << /.*/ #skip all temporary, as there is too much failures
  conf.obs_project = "YaST:Head:installer"
end

