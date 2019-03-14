#!/bin/bash
pid=$$
fifo_dir="/tmp/"
fifo_server="${fifo_dir}ledctrl"
fifo_in="${fifo_dir}ledctrl.in.${pid}"
fifo_out="${fifo_dir}ledctrl.out.${pid}"

function cleanup {
	echo "Cleanup"
	rm -f "${fifo_in}" "${fifo_out}"
}
trap cleanup EXIT


[[ -e "${fifo_in}" ]] && rm "{$fifo_in}"
[[ -e "${fifo_out}" ]] && rm "{$fifo_out}"

if ! mkfifo "${fifo_in}"; then
	echo "Failed to create ${fifo_in}"
	exit 1
fi

if ! mkfifo "${fifo_out}"; then
	echo "Failed to create ${fifo_out}"
	exit 1
fi

echo "Client pipes are created"

if [ ! -p "${fifo_server}" ]; then
	echo "Server pipe doesn't esist: ${fifo_server}"
	exit 1
fi

echo "${pid}" >"${fifo_server}"

exec 3>"${fifo_in}"
exec 4<"${fifo_out}"

while read -p "< " line; do
	command=""
	words=($line)

	if [ "${words[0]}" == "state" ]; then
		if [ -z "${words[1]}" ]; then
			command="get-led-state"
		elif [[ "${words[1]}" =~ on|off ]]; then
			command="set-led-state ${words[1]}"
		fi
	elif [ "${words[0]}" == "color" ]; then
		if [ -z "${words[1]}" ]; then
			command="get-led-color"
		elif [[ "${words[1]}" =~ red|green|blue ]]; then
			command="set-led-color ${words[1]}"
		fi
	elif [ "${words[0]}" == "rate" ]; then
		if [ -z "${words[1]}" ]; then
			command="get-led-rate"
		elif [ "${words[1]}" -ge 0 -a "${words[1]}" -le 5 ]; then
			command="set-led-rate ${words[1]}"
		fi
	fi

	if [ -z "${command}" ]; then
		echo "Supported commands: state [on|off] | color [red|green|blue] | rate [0..5]"
		continue
	fi

	echo "${command}" >&3

	read result <&4
	echo "> ${result}"
done < /dev/stdin

exec 3>&-
exec 4<&-
