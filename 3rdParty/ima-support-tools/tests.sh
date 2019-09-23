#!/usr/bin/env bats

PATH=$PWD:$PATH

export TMP_PATH="$(mktemp -d --suffix="ima-support-tests")"

setup() {
  echo "Results in ${TMP_PATH}"
}

teardown() {
  rm -rf "$TMP_PATH"
}


###########################################################################
# ima-gen-keys.sh
###########################################################################

# .system default output location
CA_CONF="$PWD/samples/ima-local-ca.genkey"
CA_KEY="$TMP_PATH/system/ima-local-ca.priv"
CA_CERT="$TMP_PATH/system/ima-local-ca.x509"

# .ima default output location
IMA_CONF="$PWD/samples/ima.genkey"
IMA_KEY="$TMP_PATH/ima/privkey_ima.pem"
IMA_CSR="$TMP_PATH/ima/csr_ima.pem"
IMA_CERT="$TMP_PATH/ima/x509_ima.pem"

@test "gen-keys: Check no args" {
  run ./ima-gen-keys.sh
  [ $status -eq 1 ]
  ./ima-gen-keys.sh | grep "Usage:"
}

@test "gen-keys: Display help" {
  ./ima-gen-keys.sh --help | grep "Usage:"
  ./ima-gen-keys.sh -h | grep "Usage:"
}

@test "gen-keys: Display version" {
  ./ima-gen-keys.sh --version | head -1 | grep version
  ./ima-gen-keys.sh -v | head -1 | grep version
}

system_gen() {

  mkdir -p "$(dirname $CA_KEY)"

  echo "Generating .system (local CA) key + cert from conf $CA_CONF"

  ./ima-gen-keys.sh -s -c "$CA_CONF" \
                       -n "$CA_KEY" \
                       -o "$CA_CERT" \
                       $HASH_OPTS \
                       $KEYLENGTH_OPTS

  # Check output
  [ -e "$CA_KEY" ]
  [ -e "$CA_CERT" ]
  openssl x509 -inform DER \
               -in "$CA_CERT" \
               -noout \
               -text
}

ima_gen() {

  mkdir -p "$(dirname $IMA_KEY)"

  echo "Generating .ima key + signing request from conf $IMA_CONF"

  ./ima-gen-keys.sh -i -m "$IMA_CONF" \
                       -p "$IMA_KEY" \
                       -q "$IMA_CSR" \
                       $KEYLENGTH_OPTS

  # Check output
  [ -e "$IMA_KEY" ]
  [ -e "$IMA_CSR" ]
}

all_gen() {

  mkdir -p "$(dirname $CA_KEY)"
  mkdir -p "$(dirname $IMA_KEY)"

  echo "Generating .system key & cert + .ima key & signing request from conf $CA_CONF & $IMA_CONF"

  ./ima-gen-keys.sh -a -c "$CA_CONF" \
                       -n "$CA_KEY" \
                       -o "$CA_CERT" \
                       -m "$IMA_CONF" \
                       -p "$IMA_KEY" \
                       -q "$IMA_CSR" \
                       $HASH_OPTS \
                       $KEYLENGTH_OPTS

  # Check output
  [ -e "$CA_KEY" ]
  [ -e "$CA_CERT" ]
  [ -e "$IMA_KEY" ]
  [ -e "$IMA_CSR" ]
}

ima_genkeys_sign_csr() {

  # Check input
  [ -e "$CA_KEY" ]
  [ -e "$CA_CERT" ]
  [ -e "$IMA_KEY" ]
  [ -e "$IMA_CSR" ]

  echo "Signing .ima CSR using .system cert & key"

  ./ima-gen-keys.sh -g -n "$CA_KEY" \
                       -o "$CA_CERT" \
                       -m "$IMA_CONF" \
                       -q "$IMA_CSR" \
                       -r "$IMA_CERT" \
                       $HASH_OPTS

  # Check output
  [ -e "$IMA_CERT" ]
  openssl x509 -inform DER \
               -in "$IMA_CERT" \
               -noout \
               -text
}

@test "gen-keys: .system key" {
  system_gen
}

@test "gen-keys: .ima key" {
  ima_gen
}

@test "gen-keys: (.system & .ima) keys + .ima sign" {
  all_gen
  ima_genkeys_sign_csr
}

@test "gen-keys: .system key + .ima key + .ima sign" {
  system_gen
  ima_gen
  ima_genkeys_sign_csr
}

# Make sure that certificates contains $accepted and not $rejected
check_certs() {
  local accepted="$1"
  local rejected="$2"

  # For .system
  echo ">>> .system"

  openssl x509 -text \
               -noout \
               -inform DER \
               -in "${CA_CERT}" | tee "${TMP_PATH}/ca.txt"

  grep "$accepted" "${TMP_PATH}/ca.txt"

  run grep "$rejected" "${TMP_PATH}/ca.txt"
  [ "$status" -eq 1 ]

  # For .ima
  echo ">>> .ima"
  openssl x509 -text \
               -noout \
               -inform DER \
               -in "${IMA_CERT}" | tee "${TMP_PATH}/ima.txt"

  grep "$accepted" "${TMP_PATH}/ima.txt"

  run grep "$rejected" "${TMP_PATH}/ima.txt"
  [ $status -eq 1 ]
}

@test "gen-keys: (.system & .ima) keys with hash parameter" {
  HASH_OPTS="-H sha512"
  all_gen
  ima_genkeys_sign_csr
  check_certs "sha512" "sha256"
}

@test "gen-keys: .system key + .ima key keys with hash parameter" {
  HASH_OPTS="-H sha512"
  system_gen
  ima_gen
  ima_genkeys_sign_csr
  check_certs "sha512" "sha256"
}

@test "gen-keys: (.system & .ima) keys with key length parameter" {
  KEYLENGTH_OPTS="-L rsa:4096"
  all_gen
  ima_genkeys_sign_csr
  check_certs "4096" "2048"
}

@test "gen-keys: .system key + .ima key keys with key length parameter" {
  KEYLENGTH_OPTS="-L rsa:4096"
  system_gen
  ima_gen
  ima_genkeys_sign_csr
  check_certs "4096" "2048"
}

###########################################################################
# ima-sign.sh
###########################################################################

gen_ima_cert() {
  all_gen
  ima_genkeys_sign_csr
}

# Generate a directory with some content
gen_content() {
  local dest=$1

  [ -n "$dest" ]

  mkdir -p "$dest"

  dd if=/dev/urandom of="${dest}/a" bs=512 count=1
  dd if=/dev/urandom of="${dest}/b" bs=512 count=1
  dd if=/dev/urandom of="${dest}/c" bs=512 count=1
}

@test "sign: Check no args" {
  run ./ima-sign.sh
  [ $status -eq 1 ]
  ./ima-sign.sh | grep "Usage:"
}

@test "sign: Display help" {
  ./ima-sign.sh | grep "Usage:"
  ./ima-sign.sh --help | grep "Usage:"
  ./ima-sign.sh -h | grep "Usage:"
}

@test "sign: Display version" {
  ./ima-sign.sh --version | head -1 | grep version
  ./ima-sign.sh -v | head -1 | grep version
}

@test "sign: Sign files (default style)" {
  gen_ima_cert

  local rdir="${TMP_PATH}/rdir"
  local dest="${TMP_PATH}/tarball.tar.bz2"
  local cnt="${TMP_PATH}/tarcontent.txt"

  gen_content "${rdir}"

  fakeroot ./ima-sign.sh -s -p "${IMA_KEY}" \
                            -d "${rdir}" \
                            -t "${dest}"

  [ -e "${dest}" ]
  bsdtar jtvf "${dest}" | tee "${cnt}"
  [ $(cat "${cnt}" | wc -l) -eq 4 ]

  echo "Check format"
  cat "${cnt}" | grep "$( basename ${rdir})/"
}

@test "sign: Sign files (legato style)" {
  gen_ima_cert

  local rdir="${TMP_PATH}/rdir"
  local dest="${TMP_PATH}/tarball.tar.bz2"
  local cnt="${TMP_PATH}/tarcontent.txt"

  gen_content "${rdir}"

  fakeroot ./ima-sign.sh -s -p "${IMA_KEY}" \
                            -d "${rdir}" \
                            -t "${dest}" \
                            -y legato

  [ -e "${dest}" ]
  bsdtar jtvf "${dest}" | tee "${cnt}"
  [ $(cat "${cnt}" | wc -l) -eq 4 ]

  echo "Check format"
  cat "${cnt}" | grep "./"
}
