/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cycle_detector.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fwuensch <fwuensch@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/02/05 12:20:46 by salquier          #+#    #+#             */
/*   Updated: 2019/03/25 15:16:27 by fwuensch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "list.h"

int        cycle_detector(const t_list *list)
{
	const t_list	*slow;
	const t_list	*fast;

	slow = list;
	fast = list;
	if (!list)
		return (0);
	while (fast && fast->next)
	{
		slow = slow->next;
		fast = fast->next->next;
		if (fast == slow)
			return (1);
	}
	return (0);
}
